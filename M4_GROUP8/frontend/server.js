const express = require('express');
const { exec } = require('child_process');
const app = express();

const PORT = 3000;

app.use(express.static('public'));

app.get('/', (res) => {
    res.sendFile('/public/index.html');
});

app.get('/features', (req) => {
    // grabs the form data from app.js
    const fileType = req.query.fileType;
    const inputFile = req.query.inputFile;
    const effectChoice = req.query.effectChoice;
    const interceptStart = req.query.interceptStart;
    const interceptLength = req.query.interceptLength;
    const speedChange = req.query.speedChange;
    const volumeMultiply = req.query.volumeMultiply;

    // uses this data to execute the equivalent of a command line argument
    // for eg its the same as the user manually going ./features 1 test.wav 0 1 0
    exec(`./features ${fileType} ${inputFile} ${effectChoice} ${interceptStart} ${interceptLength} ${speedChange} ${volumeMultiply}`, (error) => {
        if (error) {
            return error;
        }
    });
});

app.listen(PORT, () => {
    console.log(`Server is at the address: http://localhost:${PORT}`);
});
