document.addEventListener('DOMContentLoaded', loadWords);
document.getElementById('clear-all').addEventListener('click', clearAll);

function loadWords() {
  browser.storage.local.get("savedWords").then((res) => {
    const words = res.savedWords || [];
    const list = document.getElementById('word-list');
    list.innerHTML = '';

    if (words.length === 0) {
      list.innerHTML = '<div id="empty-msg">Nenhuma palavra salva ainda.</div>';
      return;
    }

    // Show newest first
    words.slice().reverse().forEach((word, index) => {
      // Index in original array is (length - 1 - index)
      const originalIndex = words.length - 1 - index;
      
      const div = document.createElement('div');
      div.className = 'word-card';
      div.innerHTML = `
        <button class="delete-btn" data-index="${originalIndex}">&times;</button>
        <div class="word-original">${escapeHtml(word.original)}</div>
        <div class="word-translated">${escapeHtml(word.translated)}</div>
        <div class="word-meta">
          <span>${word.src.toUpperCase()} &rarr; ${word.target.toUpperCase()}</span>
          <span>${new Date(word.date).toLocaleDateString()}</span>
        </div>
      `;
      list.appendChild(div);
    });

    // Add delete listeners
    document.querySelectorAll('.delete-btn').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const idx = parseInt(e.target.getAttribute('data-index'));
        deleteWord(idx);
      });
    });
  });
}

function deleteWord(index) {
  browser.storage.local.get("savedWords").then((res) => {
    const words = res.savedWords || [];
    words.splice(index, 1);
    browser.storage.local.set({ savedWords: words }).then(loadWords);
  });
}

function clearAll() {
  if (confirm("Tem certeza que deseja apagar todas as palavras?")) {
    browser.storage.local.set({ savedWords: [] }).then(loadWords);
  }
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
