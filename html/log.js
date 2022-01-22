const evtSource = new EventSource("/logging");
evtSource.onmessage = function(event) {
    const log = document.getElementById("log");
    log.textContent += event.data + "\n";
    log.scrollTop = log.scrollHeight;
}
