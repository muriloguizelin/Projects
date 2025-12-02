import requests
import json
import time
import datetime
from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.chrome.options import Options
from webdriver_manager.chrome import ChromeDriverManager

RGA = "" 
SENHA = ""
DISCORD_WEBHOOK_URL = ""
LOGIN_URL = "https://novoava.ufmt.br/graduacao/login/index.php"
AJAX_SERVICE_URL = "https://novoava.ufmt.br/graduacao/lib/ajax/service.php"

def get_session_data(rga, senha):
    print("Iniciando navegador 'invisível' (headless) para login...")
    
    options = Options()
    options.add_argument("--headless")
    options.add_argument("--no-sandbox")
    options.add_argument("--disable-dev-shm-usage")
    
    try:
        service = Service(ChromeDriverManager().install())
        driver = webdriver.Chrome(service=service, options=options)
    except Exception as e:
        print(f"Erro ao iniciar o WebDriver: {e}")
        print("Certifique-se que o Google Chrome está instalado.")
        return None, None

    try:
        driver.get(LOGIN_URL)
        
        driver.find_element(By.ID, "username").send_keys(rga)
        driver.find_element(By.ID, "password").send_keys(senha)
        driver.find_element(By.ID, "loginbtn").click()
        
        time.sleep(5) 

        if "login/index.php" in driver.current_url:
            print("ERRO: Login falhou. Verifique seu RGA/CPF e senha.")
            driver.quit()
            return None, None

        print("Login realizado com sucesso.")
        sesskey = driver.execute_script("return M.cfg.sesskey;")
        session_cookie = driver.get_cookie("MoodleSession")["value"]
        
        print("Sesskey e Cookie capturados.")
        
        driver.quit()
        return session_cookie, sesskey
        
    except Exception as e:
        print(f"Erro durante o processo de login: {e}")
        driver.quit()
        return None, None

def get_calendar_events(session_cookie, sess_key):
    print("Buscando eventos no calendário...")
    
    url = f"{AJAX_SERVICE_URL}?sesskey={sess_key}&info=core_calendar_get_calendar_monthly_view"
    
    cookies = {
        "MoodleSession": session_cookie
    }
    
    hoje = datetime.date.today()
    payload = [
        {
            "index": 0,
            "methodname": "core_calendar_get_calendar_monthly_view",
            "args": {
                "year": hoje.year,
                "month": hoje.month, 
                "courseid": 1,      
                "categoryid": 0
            }
        }
    ]
    
    try:
        response = requests.post(url, json=payload, cookies=cookies)
        response.raise_for_status()
        
        data = response.json()
        
        if data[0].get("error"):
            print(f"Erro da API Moodle: {data[0].get('exception')}")
            return None
            
        print("Eventos recebidos.")
        return data[0].get("data")
        
    except requests.exceptions.RequestException as e:
        print(f"Erro ao chamar a API do calendário: {e}")
        return None

def parse_events(data):
    if not data:
        return ["Erro ao analisar dados."], "Erro"
        
    events_list = []
    month_name = data.get("date", {}).get("month", "Mês Desconhecido")
    year = data.get("date", {}).get("year", "Ano Desconhecido")
    title = f"Próximas Entregas: {month_name} {year}"

    now_timestamp = datetime.datetime.now().timestamp()
    
    found_events = [] 

    for week in data.get("weeks", []):
        for day in week.get("days", []):
            if day.get("hasevents", False):
                for event in day.get("events", []):
                    try:
                        event_timestamp = event.get("timesort", 0)

                        if event_timestamp < now_timestamp:
                            continue  

                        event_name = event["name"]
                        event_time_html = event.get("formattedtime", "Horário não definido")
                        course_name = event["course"]["fullname"]
                        
                        event_time_clean = event_time_html.replace('<span class="dimmed_text">', '').replace('</span>', '')

                        event_date = f"{day['mday']:02d}/{data['date']['mon']:02d}/{data['date']['year']}"

                        event_name_clean = event_name.split(" é devido")[0]
                        event_name_clean = event_name_clean.split(" está marcado(a) para esta data")[0]
                        event_name_clean = event_name_clean.split(" está agendado para")[0]

                        found_events.append({
                            "timestamp": event_timestamp,
                            "date_str": event_date,
                            "time_str": event_time_clean,
                            "name": event_name_clean,
                            "course": course_name
                        })
                        
                    except Exception as e:
                        print(f"Erro ao analisar evento: {e} | Evento: {event.get('name')}")

    sorted_events = sorted(found_events, key=lambda x: x['timestamp'])

    for event in sorted_events:
        linha = f"**{event['date_str']} às {event['time_str']}**\n" \
                f"**Evento**: {event['name']}\n" \
                f"**Curso**: {event['course']}\n" \
                f"----------------------------------------"
        events_list.append(linha)

    if not events_list:
        events_list = ["Nenhuma *próxima* entrega encontrada para este mês. Parabéns!"]
        
    return events_list, title

def send_to_discord(events_list, title, webhook_url):
    print("Enviando mensagem para o Discord...")
    
    description = "\n".join(events_list)
    
    if len(description) > 4000:
        description = description[:4000] + "\n...(mensagem cortada, muitos eventos)..."

    payload = {
        "username": "Monitor AVA UFMT",
        "avatar_url": "https://www.flaticon.com/free-icon/capsicum_17858493?related_id=17858493&origin=pack", 
        "embeds": [
            {
                "title": title,
                "description": description,
                "color": 15258703, 
                "footer": {
                    "text": f"Verificação feita em: {datetime.datetime.now().strftime('%d/%m/%Y %H:%M:%S')}"
                }
            }
        ]
    }
    
    try:
        response = requests.post(webhook_url, json=payload)
        response.raise_for_status()
        print("Notificação enviada com sucesso ao Discord!")
    except requests.exceptions.RequestException as e:
        print(f"ERRO: Não foi possível enviar a mensagem para o Discord. {e}")
        print(f"Verifique se o URL do Webhook está correto: {webhook_url}")

if __name__ == "__main__":
    session_cookie, sesskey = get_session_data(RGA, SENHA)
    
    if session_cookie and sesskey:
        calendar_data = get_calendar_events(session_cookie, sesskey)
        
        if calendar_data:
            eventos, titulo = parse_events(calendar_data)
            send_to_discord(eventos, titulo, DISCORD_WEBHOOK_URL)
        else:
            print("Não foi possível obter os dados do calendário.")
    else:
        print("Não foi possível obter os dados da sessão. Encerrando.")