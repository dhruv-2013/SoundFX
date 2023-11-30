function features() {
    // grabs the form data when submit is clicked and stores them in these variables
    const fileType = document.getElementById('fileType').value;
    const inputFile = document.getElementById('inputFile').value;
    const effectChoice = document.querySelector('input[name="effectChoice"]:checked').value;
    const interceptStart = document.getElementById('interceptStart').value;
    const interceptLength = document.getElementById('interceptLength').value;
    const speedChange = document.getElementById('speedChange').value;
    const volumeMultiply = document.getElementById('volumeMultiply').value;

    // sends the data to server.js
    fetch(`/features?fileType=${fileType}&inputFile=${inputFile}&effectChoice=${effectChoice}&interceptStart=${interceptStart}&interceptLength=${interceptLength}&speedChange=${speedChange}&volumeMultiply=${volumeMultiply}`)
        .catch(error => console.error(error));
}
