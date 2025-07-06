-- CreateTable
CREATE TABLE "WifiNetwork" (
    "id" SERIAL NOT NULL,
    "ssid" TEXT,
    "bssid" TEXT NOT NULL,
    "rssi" INTEGER NOT NULL,
    "flags" TEXT NOT NULL,
    "frequency" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "locationUpdateId" INTEGER,

    CONSTRAINT "WifiNetwork_pkey" PRIMARY KEY ("id")
);

-- CreateTable
CREATE TABLE "LocationUpdate" (
    "id" SERIAL NOT NULL,
    "userAgent" TEXT NOT NULL,
    "ip" TEXT NOT NULL,
    "createdAt" TIMESTAMP(3) NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT "LocationUpdate_pkey" PRIMARY KEY ("id")
);

-- AddForeignKey
ALTER TABLE "WifiNetwork" ADD CONSTRAINT "WifiNetwork_locationUpdateId_fkey" FOREIGN KEY ("locationUpdateId") REFERENCES "LocationUpdate"("id") ON DELETE SET NULL ON UPDATE CASCADE;
