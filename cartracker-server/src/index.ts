import express from "express";
import { APIRouter } from "./routes/api-router";
import path from "path";

const app = express();

app.use("/api", APIRouter);

app.use(express.static(path.resolve("./web/"), {
  extensions: ['html']
}));

app.listen(process.env.PORT, () => {
  console.log(`Listening on http://127.0.0.1:${process.env.PORT}`);
});
