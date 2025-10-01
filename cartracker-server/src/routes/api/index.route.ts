import express from "express";
import { geolocate } from "../../geo";
import { PostDataSchema } from "../../schemas/api.schema";
import { db } from "../../db";
import { Geolocation } from "../../schemas/geo.schema";
import { authRouter } from "./auth.route";
import { locationRouter } from "./location.route";

const apiRouter = express.Router();

apiRouter.use('/auth', authRouter);
apiRouter.use('/location', locationRouter);

apiRouter.use(express.json());

apiRouter.post("/data", async (req, res) => {
  const parsed = PostDataSchema.safeParse(req.body);
  if (!parsed.success) {
    console.log("Parse error:", parsed.error);
    res.status(400).json({
      error: "failed to parse request",
    });
    return;
  }

  console.log(`Received ${parsed.data.length} location updates`);
  for (const locationUpdate of parsed.data) {
    let geo: Geolocation | undefined;
    try {
      geo = await geolocate(locationUpdate.aps);
    } catch (err) {
      console.log("Geolocation failed:", err);
    }

    try {
      await db.locationUpdate.create({
        data: {
          ip: req.ip ?? "NO IP",
          userAgent: req.header("user-agent"),
          networks: {
            createMany: {
              data: locationUpdate.aps,
            },
          },
          accuracy: geo?.accuracy,
          latitude: geo?.location.lat,
          longitude: geo?.location.lng,
        },
      });
    } catch (err) {
      console.log("Failed to insert into DB:", err);
    }
  }

  res.json({
    serverConfig: {
      interval: 10 * 60, //10 * 60, // seconds
    },
  });
});

export { apiRouter };
