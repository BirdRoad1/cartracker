import { API } from "./api.js";

const secretInput = document.getElementById("secret");
document.getElementById("submit-btn").addEventListener("click", async () => {
  const token = secretInput.value;
  if (await API.checkToken(token)) {
    localStorage.setItem("token", token);
    location.href = "/";
  } else {
    alert("Invalid token");
  }
});
