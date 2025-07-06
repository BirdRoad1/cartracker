import z from "zod";

export const WifiSchema = z.object({
  bssid: z.string().length(17),
  ssid: z.string().max(128),
  rssi: z.number().int().max(0).min(-255),
  flags: z.string().max(512),
  frequency: z.number(),
});

export const PostDataSchema = z.array(WifiSchema);