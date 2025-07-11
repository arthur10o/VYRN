async function run_code() {
    const CODE = cleanText(document.getElementById('editor').innerText);

    try {
        const RESPONSE = await fetch('/run', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ CODE })
        });

        const TEXT = await RESPONSE.text();

        if (TEXT.includes("The code has been successfully executed")) {
            displayOutput(TEXT);
        } else {
            displayError(TEXT);
        }
    } catch (err) {
        displayError("Server error: " + err.message);
    }
}

function displayError(_message) {
    const OUTPUT_DIV = document.getElementById('output');
    if (_message.includes("The code has been successfully executed")) {
        OUTPUT_DIV.style.color = '#a6e22e';
        OUTPUT_DIV.textContent = _message;
    } else {
        OUTPUT_DIV.textContent = "Error :\n" + _message;
        OUTPUT_DIV.style.color = 'red';
    }
}

function displayOutput(_output) {
    const outputDiv = document.getElementById('output');

    outputDiv.textContent = _output;
    outputDiv.style.color = '#a6e22e';
}

const EDITOR = document.getElementById('editor');

const KEYWORD = ['let', 'const', "fn"];
const TYPE = ['int', 'float', 'bool', 'string'];
const FUNCTION_CALL_PATTERN = /\b([a-zA-Z_][a-zA-Z0-9_]*)\s*(\([^)]*\))/g;
const KEYWORD_PATTERN = new RegExp(`\\b(${KEYWORD.join('|')})\\b`, 'g');
const TYPE_PATTERN = new RegExp(`\\b(${TYPE.join('|')})\\b`, 'g');
const STRING_PATTERN = /(["'])(?:(?=(\\?))\2.)*?\1/g;
const NUMBER_PATTERN = /\b\d+(\.\d+)?\b/g;
const BOOL_PATTERN = /\b(true|false)\b/g;
const COMMENT_PATTERN = /\/\*[\s\S]*?\*\//g;

let declaredVariables = new Set();
let declaredConstante = new Set();

function escapeHtml(_text) {
    return _text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;");
}

function cleanText(_text) {
    return _text.replace(/[\u200B-\u200D\uFEFF]/g, '');
}

function highlight(_text) {
    _text = escapeHtml(_text);
    declaredVariables.clear();
    declaredConstante.clear();

    const STRINGS = [];
    const COMMENTS = [];

    _text = _text.replace(STRING_PATTERN, match => {
        const PLACEHOLDER = `__STRING${STRINGS.length}__`;
        STRINGS.push(match);
        return PLACEHOLDER;
    });

    _text = _text.replace(COMMENT_PATTERN, match => {
        const PLACEHOLDER = `__COMMENT${COMMENTS.length}__`;
        COMMENTS.push(match);
        return PLACEHOLDER;
    });

    const LINES = _text.split('\n');

    LINES.forEach(line => {
        const DECLARATION_MATCH = line.match(/\b(let|const)\s+(int|float|bool|string)\s+([a-zA-Z_]\w*)\s*=/);

        if (DECLARATION_MATCH) {
            const [_, kind, type, name] = DECLARATION_MATCH;
            if (kind === 'let') {
                declaredVariables.add(name);
            } else if (kind === 'const') {
                declaredConstante.add(name);
            }
        }
    });

    const HIGHLIGHTED_LINES = LINES.map(line => {
        line = cleanText(line);

        const COMMENT_INDEX = line.indexOf('//');

        let codePart = COMMENT_INDEX >= 0 ? line.slice(0, COMMENT_INDEX) : line;
        let commentPart = COMMENT_INDEX >= 0 ? line.slice(COMMENT_INDEX) : '';

        codePart = codePart
            .replace(TYPE_PATTERN, match => `%%TYPE%%${match}%%`)
            .replace(STRING_PATTERN, match => `%%STRING%%${match}%%`)
            .replace(KEYWORD_PATTERN, match => `%%KEYWORD%%${match}%%`)
            .replace(FUNCTION_CALL_PATTERN, (match, funcName, parens) => 
                `%%FUNCTION_CALL%%<span class="function">${funcName}</span><span class="parens">${parens}</span>%%`
            )
            .replace(NUMBER_PATTERN, match => `%%NUMBER%%${match}%%`)
            .replace(BOOL_PATTERN, match => `%%BOOL%%${match}%%`);

        declaredVariables.forEach(v => {
            const VAR_PATTERN = new RegExp(`\\b${v}\\b`, 'g');
            codePart = codePart.replace(VAR_PATTERN, match => `%%VARIABLE%%${match}%%`);
        });

        declaredConstante.forEach(c => {
            const CONST_PATTERN = new RegExp(`\\b${c}\\b`, `g`);
            codePart = codePart.replace(CONST_PATTERN, match => `%%CONSTANTE%%${match}%%`);
        });

         STRINGS.forEach((str, i) => {
            const PLACEHOLDER_REGEX = new RegExp(`__STRING${i}__`, 'g');
            codePart = codePart.replace(PLACEHOLDER_REGEX, `<span class="string">${str}</span>`);
        });

        COMMENTS.forEach((comment, i) => {
            const PLACEHOLDER_REGEX = new RegExp(`__COMMENT${i}__`, 'g');
            codePart = codePart.replace(PLACEHOLDER_REGEX, `<span class="comment">${escapeHtml(comment)}</span>`);
        });

        codePart = codePart
            .replace(/%%STRING%%(.*?)%%/g, '<span class="string">$1</span>')
            .replace(/%%KEYWORD%%(.*?)%%/g, '<span class="keyword">$1</span>')
            .replace(/%%TYPE%%(.*?)%%/g, '<span class="type">$1</span>')
            .replace(/%%NUMBER%%(.*?)%%/g, '<span class="number">$1</span>')
            .replace(/%%BOOL%%(.*?)%%/g, '<span class="bool">$1</span>')
            .replace(/%%VARIABLE%%(.*?)%%/g, '<span class="variable">$1</span>')
            .replace(/%%CONSTANTE%%(.*?)%%/g, '<span class="constante">$1</span>')
            .replace(/%%FUNCTION_CALL%%(.*?)%%/g, '$1');

        if (commentPart) {
            commentPart = `<span class="comment">${escapeHtml(commentPart)}</span>`;
        }

        return codePart + commentPart;
    });

    return HIGHLIGHTED_LINES.join('<br>');
}

function saveCaretPosition(_context){
    const SELECTION = window.getSelection();
    if (SELECTION.rangeCount === 0) return 0;
    const RANGE = SELECTION.getRangeAt(0);
    const PRE_RANGE = RANGE.cloneRange();
    PRE_RANGE.selectNodeContents(_context);
    PRE_RANGE.setEnd(RANGE.startContainer, RANGE.startOffset);
    return PRE_RANGE.toString().length;
}

function restoreCaretPosition(_context, _pos) {
    let nodeStack = [_context], node, charIndex = 0, foundStart = false;

    const RANGE = document.createRange();
    RANGE.setStart(_context, 0);
    RANGE.collapse(true);

    while ((node = nodeStack.pop()) && !foundStart) {
        if (node.nodeType === 3) {
            const NEXT_CHAR_INDEX = charIndex + node.length;
            if (_pos >= charIndex && _pos <= NEXT_CHAR_INDEX) {
                RANGE.setStart(node, _pos - charIndex);
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

function getCaretCharacterOffsetWithin(_element) {
    const SEL = window.getSelection();
    let caretOffset = 0;

    if (SEL.rangeCount > 0) {
        const RANGE = SEL.getRangeAt(0);
        const PRE_CARET_RANGE = RANGE.cloneRange();
        PRE_CARET_RANGE.selectNodeContents(_element);
        PRE_CARET_RANGE.setEnd(RANGE.endContainer, RANGE.endOffset);
        caretOffset = PRE_CARET_RANGE.toString().length;
    }

    return caretOffset;
}

function setCaretPosition(_element, _offset) {
    let charIndex = 0;
    const RANGE = document.createRange();

    RANGE.setStart(_element, 0);
    RANGE.collapse(true);

    const NODE_STACK = [_element];
    let node, found = false;

    while (!found && (node = NODE_STACK.pop())) {
        if (node.nodeType === 3) {
            const NEXT_CHAR_INDEX = charIndex + node.length;
            if (_offset <= NEXT_CHAR_INDEX) {
                RANGE.setStart(node, _offset - charIndex);
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

function getPlainTextWithLineBreaks(_element) {
    let html = _element.innerHTML;

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
        const SELECTION = window.getSelection();
        const RANGE = SELECTION.getRangeAt(0);
        RANGE.deleteContents();

        const BR = document.createElement('br');
        const TEXT_NODE = document.createTextNode('\u200B');

        RANGE.insertNode(BR);
        RANGE.setStartAfter(BR);
        RANGE.insertNode(TEXT_NODE);
        range.setStart(TEXT_NODE, 1);
        range.collapse(true);

        SELECTION.removeAllRanges();
        SELECTION.addRange(RANGE);

        const RAW_TEXT = getPlainTextWithLineBreaks(EDITOR);
        const CARET_POS = getCaretCharacterOffsetWithin(EDITOR);
        const HIGHLIGHTED = highlight(RAW_TEXT);
        EDITOR.innerHTML = HIGHLIGHTED;
        setCaretPosition(EDITOR, CARET_POS);

        return;
    }
});