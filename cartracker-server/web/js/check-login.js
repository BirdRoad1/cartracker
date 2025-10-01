import { API } from "./api.js";

(async () => {
  const token = localStorage.getItem("token");
  if (token === null) {
    location.href = "/login";
    return;
  }

  if (!(await API.checkToken(token))) {
    location.href = "/login";
  }
})();
