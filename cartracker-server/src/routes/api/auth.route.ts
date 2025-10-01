import express from "express";
import { authMiddleware } from "../../middleware/auth.middleware";

const authRouter = express.Router();

authRouter.use(express.json());
authRouter.post(
  "/verify-token",
  authMiddleware(),
  (
    req, res
  ) => {
    res.json({});
  }
);

export { authRouter };
