import express from "express";
import { authMiddleware } from "../../middleware/auth.middleware";
import { db } from "../../db";

const locationRouter = express.Router();

locationRouter.use(authMiddleware());
locationRouter.get("/history", async (req, res) => {
  const results = await db.locationUpdate.findMany({
    orderBy: {
      createdAt: "desc",
    },
    take: 50,
  });

  res.json({ results });
});

export { locationRouter };
