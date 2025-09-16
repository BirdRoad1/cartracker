import type { RequestHandler } from 'express';
import { User } from '../repository/user.repository.js';
import jwt from '../lib/jwt.js';

interface Options {
  requiresLogIn?: boolean;
  adminOnly?: boolean;
}

export function authMiddleware(options?: Options): RequestHandler {
  return async (req, res, next) => {
    const header = req.header('authorization');
    if (!header) {
      if (options?.requiresLogIn || options?.adminOnly) {
        return res
          .status(401)
          .json({ message: 'Authentication is required for this endpoint' });
      } else {
        return next();
      }
    }

    const split = header.split(' ');
    const token = split[1];
    if (token === undefined || split[0]?.toLowerCase() !== 'bearer') {
      return res
        .status(401)
        .json({ message: 'Only Bearer authentication is supported' });
    }

    let data;
    try {
      data = jwt.verify(token);
    } catch (err) {
      console.log('JWT verification failed:', req.ip, err);
      return res.status(401).json({ message: 'Invalid token' });
    }

    const user = await User.get(data.id, true);
    if (!user) {
      return res.status(401).json({
        message: 'Invalid session'
      });
    }

    if (options?.adminOnly && !user.isAdmin) {
      return res
        .status(403)
        .json({ message: 'You are not authorized to access this resource' });
    }

    res.locals.user = user;
    next();
  };
}