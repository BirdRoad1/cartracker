import { API } from "./api.js";

const submitBtn = document.getElementById('submit-btn');
const intervalText = document.getElementById('interval');

submitBtn.addEventListener('click', async () => {
  const interval = Number(intervalText.value);
  if (!Number.isFinite(interval)) {
    return alert('Invalid interval');
  }
  
  try {
    await API.updateSettings(interval);
    alert('Settings saved!');
  } catch (err) {
    alert(err.message);
  }
});