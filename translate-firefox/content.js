let modal = null;

browser.runtime.onMessage.addListener((request, sender, sendResponse) => {
  if (request.action === "translate") {
    const text = request.text || window.getSelection().toString().trim();
    if (text) {
      handleTranslation(text);
    }
  }
});

async function handleTranslation(text) {
  removeModal(); // Remove existing if any
  createModal();
  showLoading();

  try {
    // Step 1: Try translating to Portuguese first to detect language
    let result = await fetchTranslation(text, 'pt');
    
    // Step 2: If detected language is Portuguese, translate to English
    if (result.src === 'pt') {
      result = await fetchTranslation(text, 'en');
    }

    showResult(result);
  } catch (error) {
    showError("Erro na tradução: " + error.message);
  }
}

async function fetchTranslation(text, targetLang) {
  const url = `https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=${targetLang}&dt=t&q=${encodeURIComponent(text)}`;
  const response = await fetch(url);
  const data = await response.json();
  
  // data[0][0][0] => translated text
  // data[2] => detected language code (sometimes)
  // Or data[8][0][0] for language? 
  // The structure is complex. Usually:
  // data[0] is array of sentences.
  // data[2] is the detected source language (e.g., "en").
  
  const translatedText = data[0].map(item => item[0]).join('');
  const detectedLang = data[2];

  return {
    original: text,
    translated: translatedText,
    src: detectedLang,
    target: targetLang
  };
}

function createModal() {
  modal = document.createElement('div');
  modal.id = 'zen-translate-modal';
  
  // Position modal near top right or center
  modal.style.top = '20px';
  modal.style.right = '20px';
  
  document.body.appendChild(modal);
}

function removeModal() {
  if (modal) {
    modal.remove();
    modal = null;
  }
}

function showLoading() {
  if (!modal) return;
  modal.innerHTML = `
    <div id="zen-translate-header">
      <span id="zen-translate-title">Traduzindo...</span>
      <button id="zen-translate-close">&times;</button>
    </div>
    <div class="zen-translate-loading">Carregando tradução...</div>
  `;
  
  document.getElementById('zen-translate-close').onclick = removeModal;
}

function showResult(data) {
  if (!modal) return;
  
  const srcLabel = data.src.toUpperCase();
  const targetLabel = data.target.toUpperCase();

  modal.innerHTML = `
    <div id="zen-translate-header">
      <span id="zen-translate-title">Tradução (${srcLabel} &rarr; ${targetLabel})</span>
      <button id="zen-translate-close">&times;</button>
    </div>
    
    <div class="zen-translate-row">
      <div class="zen-translate-label">Original</div>
      <div class="zen-translate-text">${escapeHtml(data.original)}</div>
    </div>
    
    <div class="zen-translate-row">
      <div class="zen-translate-label">Tradução</div>
      <div class="zen-translate-text">${escapeHtml(data.translated)}</div>
    </div>

    <div id="zen-translate-actions">
      <button id="zen-translate-save" class="zen-translate-btn">Salvar Palavra</button>
    </div>
  `;

  document.getElementById('zen-translate-close').onclick = removeModal;
  
  const saveBtn = document.getElementById('zen-translate-save');
  saveBtn.onclick = () => {
    saveWord(data);
    saveBtn.textContent = "Salvo!";
    saveBtn.disabled = true;
    
    // Add a helper message
    const actionsDiv = document.getElementById('zen-translate-actions');
    const hint = document.createElement('div');
    hint.style.fontSize = '11px';
    hint.style.color = '#a6e3a1';
    hint.style.marginTop = '8px';
    hint.style.textAlign = 'right';
    hint.innerText = 'Clique no ícone da extensão para ver sua lista.';
    actionsDiv.parentNode.insertBefore(hint, actionsDiv.nextSibling);
  };
}

function showError(msg) {
  if (!modal) return;
  modal.innerHTML = `
    <div id="zen-translate-header">
      <span id="zen-translate-title">Erro</span>
      <button id="zen-translate-close">&times;</button>
    </div>
    <div style="color: #f38ba8;">${escapeHtml(msg)}</div>
  `;
  document.getElementById('zen-translate-close').onclick = removeModal;
}

function saveWord(data) {
  browser.storage.local.get("savedWords").then((res) => {
    const words = res.savedWords || [];
    words.push({
      original: data.original,
      translated: data.translated,
      src: data.src,
      target: data.target,
      date: new Date().toISOString()
    });
    browser.storage.local.set({ savedWords: words });
  });
}

function escapeHtml(text) {
  if (!text) return "";
  return text
    .replace(/&/g, "&amp;")
    .replace(/</g, "&lt;")
    .replace(/>/g, "&gt;")
    .replace(/"/g, "&quot;")
    .replace(/'/g, "&#039;");
}
