// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

window.addEventListener('load', () => {
    initWebSocket();
    setTimeout(() => {
        renderGauges();
    }, 500);
});

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection…');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function Send_Data(data) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(data);
        console.log("📤 Gửi:", data);
    } else {
        console.warn("⚠️ WebSocket chưa sẵn sàng!");
        alert("⚠️ WebSocket chưa kết nối!");
    }
}

let gaugeTemp = null;
let gaugeHumi = null;

function onMessage(event) {
    console.log("📩 Nhận:", event.data);
    try {
        var data = JSON.parse(event.data);
        if (data.page === "home" && data.value) {
            if (data.value.temperature !== undefined && gaugeTemp !== null) {
                gaugeTemp.refresh(data.value.temperature);
            }
            if (data.value.humidity !== undefined && gaugeHumi !== null) {
                gaugeHumi.refresh(data.value.humidity);
            }
        }
    } catch (e) {
        console.warn("Không phải JSON hợp lệ:", event.data);
    }
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}

function toggleSidebar() {
    const sidebar = document.getElementById('sidebar');
    const logoImg = document.getElementById('sidebarLogo');
    if (sidebar) {
        sidebar.classList.toggle('collapsed');
        if (sidebar.classList.contains('collapsed')) {
            logoImg.src = 'logo_org1.png'; 
        } else {
            logoImg.src = 'logo_org.png';
        }
    }
}

// ==================== THEME TOGGLE ====================
document.addEventListener('DOMContentLoaded', () => {
    const themeBtn = document.getElementById('themeToggle');
    const savedTheme = localStorage.getItem('theme');

    if (savedTheme) {
        document.body.setAttribute('data-theme', savedTheme);
        updateThemeToggleUI(savedTheme);
    }

    if(themeBtn) {
        themeBtn.addEventListener('click', () => {
            const currentTheme = document.body.getAttribute('data-theme');
            const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
            document.body.setAttribute('data-theme', newTheme);
            localStorage.setItem('theme', newTheme);
            updateThemeToggleUI(newTheme);

            document.getElementById("gauge_temp").innerHTML = "";
            document.getElementById("gauge_humi").innerHTML = "";
            renderGauges();
        });
    }
});

function updateThemeToggleUI(theme) {
    const themeBtn = document.getElementById('themeToggle');
    if(!themeBtn) return;
    const icon = themeBtn.querySelector('i');
    const text = themeBtn.querySelector('span');
    
    if (theme === 'dark') {
        icon.className = 'fa-solid fa-sun';
        text.textContent = 'Sáng';
    } else {
        icon.className = 'fa-solid fa-moon';
        text.textContent = 'Tối';
    }
}


// ==================== HOME GAUGES ====================
function renderGauges() {
    const currentTheme = document.body.getAttribute('data-theme') || 'light';
    const textColor = (currentTheme === 'dark') ? "#ffffff" : "#0f172a";

    const tempContainer = document.getElementById("gauge_temp");
    const humiContainer = document.getElementById("gauge_humi");
    if(tempContainer) tempContainer.innerHTML = "";
    if(humiContainer) humiContainer.innerHTML = "";

    const commonConfig = {
        value: 0, 
        donut: true,
        pointer: false,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        gaugeWidthScale: 0.25,
        valueFontColor: textColor, 
        titleFontColor: textColor,
        startAnimationTime: 0, 
        refreshAnimationTime: 0,
        showInnerShadow: true,
        shadowOpacity: 0.5
    };

    gaugeTemp = new JustGage({
        id: "gauge_temp",
        min: -10,
        max: 50,
        title: "Nhiệt độ",
        label: "\n°C",
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"],
        ...commonConfig
    });

    gaugeHumi = new JustGage({
        id: "gauge_humi",
        min: 0,
        max: 100,
        title: "Độ ẩm",
        label: "\n%",
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"],
        ...commonConfig
    });
}

// ==================== DEVICE FUNCTIONS ====================
function openAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'flex';
}
function closeAddRelayDialog() {
    document.getElementById('addRelayDialog').style.display = 'none';
}
function saveRelay() {
    const name = document.getElementById('relayName').value.trim();
    const gpio = document.getElementById('relayGPIO').value.trim();
    if (!name || !gpio) return alert("⚠️ Vui lòng nhập đủ thông tin!");
    relayList.push({ id: Date.now(), name, gpio, state: false });
    renderRelays();
    closeAddRelayDialog();
}
function renderRelays() {
    const container = document.getElementById('relayContainer');
    container.innerHTML = "";
    relayList.forEach(r => {
        const card = document.createElement('div');
        card.className = 'device-card';
        card.innerHTML = `
      <i class="fa-solid fa-bolt device-icon"></i>
      <h3>${r.name}</h3>
      <p>GPIO: ${r.gpio}</p>
      <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
        ${r.state ? 'ON' : 'OFF'}
      </button>
      <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
    `;
        container.appendChild(card);
    });
}
function toggleRelay(id) {
    const relay = relayList.find(r => r.id === id);
    if (relay) {
        relay.state = !relay.state;
        const relayJSON = JSON.stringify({
            page: "device",
            value: {
                name: relay.name,
                status: relay.state ? "ON" : "OFF",
                gpio: relay.gpio
            }
        });
        Send_Data(relayJSON);
        renderRelays();
    }
}
function showDeleteDialog(id) {
    deleteTarget = id;
    document.getElementById('confirmDeleteDialog').style.display = 'flex';
}
function closeConfirmDelete() {
    document.getElementById('confirmDeleteDialog').style.display = 'none';
}
function confirmDelete() {
    relayList = relayList.filter(r => r.id !== deleteTarget);
    renderRelays();
    closeConfirmDelete();
}


// ==================== SETTINGS FORM (BỔ SUNG) ====================
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const password = document.getElementById("password").value.trim();
    const token = document.getElementById("token").value.trim();
    const server = document.getElementById("server").value.trim();
    const port = document.getElementById("port").value.trim();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: ssid,
            password: password,
            token: token,
            server: server,
            port: port
        }
    });

    Send_Data(settingsJSON);
    alert("✅ Cấu hình đã được gửi đến thiết bị!");
});