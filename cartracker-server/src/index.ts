import express from "express";
import { apiRouter } from "./routes/api/index.route";
import path from "path";
import { env } from "./env/env";

const app = express();

app.use("/api/v1/", apiRouter);

app.use(express.static(path.resolve("./web/"), {
  extensions: ['html']
}));

app.listen(env.PORT, () => {
  console.log(`Listening on http://127.0.0.1:${env.PORT}`);
});
