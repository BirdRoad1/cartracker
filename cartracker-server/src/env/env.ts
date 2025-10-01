import z from "zod";
import { envSchema } from "./env.schema.js";

try {
  await import("dotenv/config");
  console.log("Imported .env file");
} catch { /* empty */ }

const parsedEnv = envSchema.safeParse(process.env);
if (!parsedEnv.success) {
  console.log("Invalid environment variables:\n" + z.prettifyError(parsedEnv.error));
  process.exit(1);
}

export const env = parsedEnv.data;