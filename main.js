function run_code() {
    const CODE = document.getElementById('editor').value;

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