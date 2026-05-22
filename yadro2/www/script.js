// Графики
const cpuCtx = document.getElementById('cpuChart').getContext('2d');
const ramCtx = document.getElementById('ramChart').getContext('2d');
let cpuChart = new Chart(cpuCtx, {
    type: 'line', data: { labels: [], datasets: [{ label: 'CPU %', data: [], borderColor: '#e5c07b', tension: 0.2, fill: false }] },
    options: { responsive: true, maintainAspectRatio: true }
});
let ramChart = new Chart(ramCtx, {
    type: 'line', data: { labels: [], datasets: [{ label: 'RAM %', data: [], borderColor: '#61afef', tension: 0.2, fill: false }] },
    options: { responsive: true, maintainAspectRatio: true }
});

function updateChart(chart, label, value) {
    if (chart.data.labels.length > 60) { chart.data.labels.shift(); chart.data.datasets[0].data.shift(); }
    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(value);
    chart.update();
}

async function fetchStats() {
    const res = await fetch('/stats');
    const data = await res.json();
    const now = new Date(data.timestamp * 1000).toLocaleTimeString();
    updateChart(cpuChart, now, data.cpu);
    updateChart(ramChart, now, data.ram);
    document.getElementById('cpu-val').innerText = data.cpu.toFixed(1);
    document.getElementById('ram-val').innerText = data.ram.toFixed(1);
    document.getElementById('temp-val').innerText = data.cpu_temp.toFixed(1);
    
    let disksHtml = '<ul style="margin:0">';
    data.disks.forEach(d => { disksHtml += `<li>${d.mount}: ${d.usage.toFixed(1)}%</li>`; });
    disksHtml += '</ul>';
    document.getElementById('disks-list').innerHTML = disksHtml;
}

let processes = [];
let sortBy = 'cpu'; let sortAsc = false;

function renderTable() {
    const tbody = document.querySelector('#process-table tbody');
    tbody.innerHTML = '';
    const sorted = [...processes].sort((a,b) => {
        let valA = a[sortBy], valB = b[sortBy];
        if (sortBy === 'name') valA = valA.toLowerCase(), valB = valB.toLowerCase();
        if (typeof valA === 'string') return sortAsc ? valA.localeCompare(valB) : valB.localeCompare(valA);
        return sortAsc ? valA - valB : valB - valA;
    });
    for (let p of sorted) {
        const row = tbody.insertRow();
        row.insertCell(0).innerText = p.pid;
        row.insertCell(1).innerText = p.name;
        row.insertCell(2).innerText = p.cpu.toFixed(1);
        row.insertCell(3).innerText = p.mem_mb;
        }
}

async function fetchProcesses() {
    try {
        const res = await fetch('/processes');
        processes = await res.json();
        renderTable();
    } catch(e) { console.error(e); }
}


// Сортировка по клику на заголовок
document.querySelectorAll('#process-table th').forEach(th => {
    th.addEventListener('click', () => {
        const key = th.dataset.sort;
        if (sortBy === key) sortAsc = !sortAsc;
        else { sortBy = key; sortAsc = false; }
        renderTable();
    });
});

setInterval(fetchStats, 1000);
setInterval(fetchProcesses, 2000);
fetchStats();
fetchProcesses();