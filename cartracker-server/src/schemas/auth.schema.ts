import z from "zod";

export const authSchema = z.object({
  token: z.string(),
});
