import z from 'zod';

export const envSchema = z.object({
  PORT: z
    .string()
    .default('8000')
    .transform(z => Number(z))
    .refine(n => n >= 0 && n <= 65535, { message: 'Invalid port number' }),
  NODE_ENV: z.enum(['development', 'production']).default('development'),
  DATABASE_URL: z.string(),
  TOKEN: z.string(),
  TRUST_PROXY: z
    .string()
    .default('false')
    .transform(s => s === 'true')
});