import type { RequestHandler } from "express";
import { env } from "../env/env";

export function authMiddleware(): RequestHandler {
  return async (req, res, next) => {
    const header = req.header("authorization");
    if (!header) {
      return res
        .status(401)
        .json({ message: "Authentication is required for this endpoint" });
    }

    const split = header.split(" ");
    const token = split[1];
    if (token === undefined || split[0]?.toLowerCase() !== "bearer") {
      return res
        .status(401)
        .json({ message: "Only Bearer authentication is supported" });
    }

    if (token != env.TOKEN) {
      return res.status(401).json({
        message: "Invalid token",
      });
    }
    next();
  };
}
