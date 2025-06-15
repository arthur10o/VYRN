function run_code() {
    const CODE = document.getElementById('editor').innerText;

    fetch('/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ code: CODE })
    })
    .then(response => {
        if (!response.ok) throw new Error(`Erreur HTTP ${response.status}`);
        return response.text();
    })
    .then(result => {
        document.getElementById('output').textContent = result;
    })
    .catch(err => {
        console.error(err);
        document.getElementById('output').textContent = "Erreur : " + err;
    });
}

const EDITOR = document.getElementById('editor');

const KEYWORD = ['let', 'print'];
const KEYWORD_PATTERN = new RegExp(`\\b(${KEYWORD.join('|')})\\b`, 'g');
const STRING_PATTERN = /(["'])(?:(?=(\\?))\2.)*?\1/g;
const NUMBER_PATTERN = /\b\d+(\.\d+)?\b/g;
const BOOL_PATTERN = /\b(true|false)\b/g;
const COMMENT_PATTERN = /\/\/.*/g;

function escapeHtml(text) {
    return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

function highlight(text) {
    text = escapeHtml(text);

    text = text.replace(COMMENT_PATTERN, match => `%%COMMENT%%${match}%%`);
    text = text.replace(STRING_PATTERN, match => `%%STRING%%${match}%%`);
    text = text.replace(KEYWORD_PATTERN, match => `%%KEYWORD%%${match}%%`);
    text = text.replace(NUMBER_PATTERN, match => `%%NUMBER%%${match}%%`);
    text = text.replace(BOOL_PATTERN, match => `%%BOOL%%${match}%%`);

    text = text.replace(/%%COMMENT%%(.*?)%%/g, '<span class="comment">$1</span>');
    text = text.replace(/%%STRING%%(.*?)%%/g, '<span class="string">$1</span>');
    text = text.replace(/%%KEYWORD%%(.*?)%%/g, '<span class="keyword">$1</span>');
    text = text.replace(/%%NUMBER%%(.*?)%%/g, '<span class="number">$1</span>');
    text = text.replace(/%%BOOL%%(.*?)%%/g, '<span class="bool">$1</span>');

    text = text.replace(/\n/g, '<br>');

    return text;
}

function saveCaretPosition(context){
    const SELECTION = window.getSelection();
    if (SELECTION.rangeCount === 0) return 0;
    const RANGE = SELECTION.getRangeAt(0);
    const PRE_RANGE = RANGE.cloneRange();
    PRE_RANGE.selectNodeContents(context);
    PRE_RANGE.setEnd(RANGE.startContainer, RANGE.startOffset);
    return PRE_RANGE.toString().length;
}

function restoreCaretPosition(context, pos) {
    let nodeStack = [context], node, charIndex = 0, foundStart = false;
    const RANGE = document.createRange();
    RANGE.setStart(context, 0);
    RANGE.collapse(true);

    while ((node = nodeStack.pop()) && !foundStart) {
        if (node.nodeType === 3) {
            const NEXT_CHAR_INDEX = charIndex + node.length;
            if (pos >= charIndex && pos <= NEXT_CHAR_INDEX) {
                RANGE.setStart(node, pos - charIndex);
                RANGE.collapse(true);
                foundStart = true;
            }
            charIndex = NEXT_CHAR_INDEX;
        } else {
            let i = node.childNodes.length;
            while (i--) nodeStack.push(node.childNodes[i]);
        }
    }

    const SEL = window.getSelection();
    SEL.removeAllRanges();
    SEL.addRange(RANGE);
}

function getCaretCharacterOffsetWithin(element) {
    const SEL = window.getSelection();
    let caretOffset = 0;
    if (SEL.rangeCount > 0) {
        const RANGE = SEL.getRangeAt(0);
        const PRE_CARET_RANGE = RANGE.cloneRange();
        PRE_CARET_RANGE.selectNodeContents(element);
        PRE_CARET_RANGE.setEnd(RANGE.endContainer, RANGE.endOffset);
        caretOffset = PRE_CARET_RANGE.toString().length;
    }
    return caretOffset;
}

function setCaretPosition(element, offset) {
    let charIndex = 0;
    const RANGE = document.createRange();
    RANGE.setStart(element, 0);
    RANGE.collapse(true);

    const NODE_STACK = [element];
    let node, found = false;

    while (!found && (node = NODE_STACK.pop())) {
        if (node.nodeType === 3) {
            const NEXT_CHAR_INDEX = charIndex + node.length;
            if (offset <= NEXT_CHAR_INDEX) {
                RANGE.setStart(node, offset - charIndex);
                RANGE.collapse(true);
                found = true;
            } else {
                charIndex = NEXT_CHAR_INDEX;
            }
        } else {
            let i = node.childNodes.length;
            while (i--) {
                NODE_STACK.push(node.childNodes[i]);
            }
        }
    }

    const SEL = window.getSelection();
    SEL.removeAllRanges();
    SEL.addRange(RANGE);
}

function getPlainTextWithLineBreaks(element) {
    let html = element.innerHTML;

    html = html.replace(/<br\s*\/?>/gi, '\n');

    const TEMP_DIV = document.createElement('div');
    TEMP_DIV.innerHTML = html;
    return TEMP_DIV.textContent;
}

EDITOR.addEventListener('input', () => {
    const CARET_POS = saveCaretPosition(EDITOR);
    const RAW_TEXT = EDITOR.innerText;
    const HIGHLIGHTED = highlight(RAW_TEXT);
    EDITOR.innerHTML = HIGHLIGHTED;
    restoreCaretPosition(EDITOR, CARET_POS);
});

EDITOR.addEventListener('keydown', e => {
    if (e.key === 'Enter') {
        e.preventDefault();

        const selection = window.getSelection();
        if (!selection.rangeCount) return;

        const range = selection.getRangeAt(0);
        range.deleteContents();

        const br = document.createElement('br');
        const textNode = document.createTextNode('\u200B');

        range.insertNode(br);
        range.setStartAfter(br);
        range.insertNode(textNode);
        range.setStart(textNode, 1);
        range.collapse(true);

        selection.removeAllRanges();
        selection.addRange(range);

        const RAW_TEXT = getPlainTextWithLineBreaks(EDITOR);
        const CARET_POS = getCaretCharacterOffsetWithin(EDITOR);
        const HIGHLIGHTED = highlight(RAW_TEXT);
        EDITOR.innerHTML = HIGHLIGHTED;
        setCaretPosition(EDITOR, CARET_POS);

        return;
    }
});