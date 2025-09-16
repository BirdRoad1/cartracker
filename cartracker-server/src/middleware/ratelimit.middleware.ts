import rateLimit from 'express-rate-limit';
import { env } from '../env/env.js';

export const ratelimit = rateLimit({
  windowMs: 1000,
  limit: 5,
  ipv6Subnet: 56,
  validate: { trustProxy: !env.TRUST_PROXY }
});