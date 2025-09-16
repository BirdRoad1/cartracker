import type { RequestHandler } from "express";
import z, { ZodType } from "zod";
import type { users } from "../db/schema.js";

export type RequestHandlerWithBody<S extends ZodType = never> = RequestHandler<
  Record<string, string>,
  unknown,
  z.infer<S>,
  unknown,
  {
    user?: typeof users.$inferSelect;
  }
>;

export function validateData(schema: ZodType): RequestHandler {
  return (req, res, next) => {
    const result = schema.safeParse(req.body);
    if (!result.success) {
      return res.status(400).json({ message: result.error.issues[0]?.message });
    }

    next();
  };
}