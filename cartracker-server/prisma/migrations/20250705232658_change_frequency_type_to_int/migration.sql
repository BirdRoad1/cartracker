/*
  Warnings:

  - Changed the type of `frequency` on the `WifiNetwork` table. No cast exists, the column would be dropped and recreated, which cannot be done if there is data, since the column is required.

*/
-- AlterTable
ALTER TABLE "WifiNetwork" DROP COLUMN "frequency",
ADD COLUMN     "frequency" INTEGER NOT NULL;
