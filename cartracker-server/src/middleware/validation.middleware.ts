import type { RequestHandler } from "express";
import z, { ZodType } from "zod";

export function validateData(schema: ZodType): RequestHandler {
  return (req, res, next) => {
    const result = schema.safeParse(req.body);
    if (!result.success) {
      return res.status(400).json({ message: result.error.issues[0]?.message });
    }

    next();
  };
}