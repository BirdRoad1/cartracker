import z from "zod";

export const GeolocationSchema = z.object({
  location: z.object({
    lat: z.number(),
    lng: z.number(),
  }),
  accuracy: z.number(),
});

export type Geolocation = z.infer<typeof GeolocationSchema>;
