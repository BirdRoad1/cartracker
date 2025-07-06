import express from "express";
import { APIRouter } from "./routes/api-router";

const app = express();

app.use("/api", APIRouter);

app.listen(8000, () => {
  console.log("Listening on http://127.0.0.1:8000/");
});
