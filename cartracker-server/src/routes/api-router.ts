import express from "express";
import { geolocate } from "../geo";
import { PostDataSchema, WifiSchema } from "../schemas/api-schema";
import { db } from "../db";
import { Geolocation } from "../schemas/geo-schema";

const APIRouter = express.Router();

APIRouter.use(express.json());

APIRouter.post("/data", async (req, res) => {
  const parsed = PostDataSchema.safeParse(req.body);
  if (!parsed.success) {
    console.log("Parse error:", parsed.error);
    res.status(400).json({
      error: "failed to parse request",
    });
    return;
  }
  const { data: networks } = parsed;

  let geo: Geolocation | undefined;
  try {
    geo = await geolocate(networks);
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
            data: networks,
          },
        },
        accuracy: geo?.accuracy,
        latitude: geo?.location.lat,
        longitude: geo?.location.lng
      },
    });
  } catch (err) {
    console.log("Failed to insert into DB:", err);
  }

  console.log("Location update received!");
  res.json({});
});

export { APIRouter };
