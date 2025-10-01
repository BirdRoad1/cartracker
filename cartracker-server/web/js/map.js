import { API } from "./api.js";

const demoWarning = document.getElementById("demo-warning");
if (localStorage.getItem("token") === "demo") {
  demoWarning.style.display = "block";
}

const locationUpdatesDiv = document.getElementById("location-updates");

function clearLocationUpdates() {
  locationUpdatesDiv.replaceChildren();
}

function addLocationUpdateElem(software, date) {
  const div = document.createElement("div");
  div.classList.add("location-update");
  
  const softwareLine = document.createElement("p");
  softwareLine.textContent = 'Software: ' + software;
  div.appendChild(softwareLine);

  const dateLine = document.createElement("p");
  dateLine.textContent = 'Date: ' + date.toLocaleDateString();
  div.appendChild(dateLine);

  const timeLine = document.createElement("p");
  timeLine.textContent = 'Time: ' + date.toLocaleTimeString();
  div.appendChild(timeLine);

  locationUpdatesDiv.appendChild(div);
}

(async () => {
  const locationHistory = await API.getLocationHistory();
  console.log(locationHistory);
  if (locationHistory.length === 0) {
    alert("No location history found");
    return;
  }

  const first = locationHistory[0];
  console.log([first.latitude, first.longitude]);
  const map = L.map("map", {
    center: [first.latitude, first.longitude],
    zoom: 16,
  });

  const coords = locationHistory.map((l) => [l.latitude, l.longitude]);
  L.polyline(coords, { color: "#000000" }).addTo(map);

  for (let i = 0; i < locationHistory.length; i++) {
    const location = locationHistory[i];
    const date = new Date(location.createdAt);
    const marker = L.marker([location.latitude, location.longitude], {
      title: `${date.toLocaleDateString()} ${date.toLocaleTimeString()}`
    }).addTo(map);

    const circle = L.circle([location.latitude, location.longitude], {
      radius: location.accuracy,
    }).addTo(map);
    console.log(circle);
    if (i === 0) {
      marker._icon.classList.add("hue");
      circle._path.classList.add("hue");
    }

    addLocationUpdateElem(location.userAgent, date);
  }
  //.setView([51.505, -0.09], 13);

  L.tileLayer("https://tile.openstreetmap.org/{z}/{x}/{y}.png", {
    maxZoom: 30,
    attribution:
      '&copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>',
  }).addTo(map);
})();
