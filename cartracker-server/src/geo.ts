import "dotenv/config";
import z from "zod";
import { ApSchema } from "./schemas/api-schema";
import { Geolocation, GeolocationSchema } from "./schemas/geo-schema";

type WifiNetwork = z.infer<typeof ApSchema>;

function getChannel(frequency: number) {
  return Math.floor((frequency - 2407) / 5);
}

export async function geolocate(
  wifis: WifiNetwork[]
): Promise<Geolocation> {
  const url = `https://www.googleapis.com/geolocation/v1/geolocate?key=${process.env.API_KEY}`;
  const body = {
    considerIp: false,
    wifiAccessPoints: wifis.map((wifi) => ({
      macAddress: wifi.bssid,
      signalStrength: wifi.rssi,
      channel: getChannel(wifi.frequency),
    })),
  };

  const res = await fetch(url, {
    method: "POST",
    headers: {
      "content-type": "application/json",
    },
    body: JSON.stringify(body),
  });

  if (!res.ok) {
    throw new Error(await res.text());
  }

  const json = await res.json();

  const parsed = GeolocationSchema.safeParse(json);
  if (!parsed.success) {
    throw new Error("Failed to parse geolocation response: " + JSON.stringify(json) + ", errors: " + parsed.error);
  }

  return parsed.data;
}
