browser.contextMenus.create({
  id: "translate-selection",
  title: "Traduzir e Salvar",
  contexts: ["selection"]
});

// Helper to safely send message or inject scripts
async function sendMessageToTab(tabId, message) {
  try {
    await browser.tabs.sendMessage(tabId, message);
  } catch (error) {
    console.warn("Script de conteúdo não encontrado. Tentando injetar...", error);
    try {
      await browser.scripting.insertCSS({
        target: { tabId: tabId },
        files: ["styles.css"]
      });
      await browser.scripting.executeScript({
        target: { tabId: tabId },
        files: ["content.js"]
      });
      await browser.tabs.sendMessage(tabId, message);
    } catch (injectError) {
      console.error("Falha ao injetar script ou reenviar mensagem:", injectError);
    }
  }
}

browser.contextMenus.onClicked.addListener((info, tab) => {
  if (info.menuItemId === "translate-selection") {
    sendMessageToTab(tab.id, {
      action: "translate",
      text: info.selectionText
    });
  }
});

browser.commands.onCommand.addListener((command) => {
  if (command === "translate-selection-command") {
    browser.tabs.query({active: true, currentWindow: true}).then((tabs) => {
      if (tabs.length > 0) {
        sendMessageToTab(tabs[0].id, { action: "translate" }); // No text provided, content script will grab it
      }
    });
  }
});
