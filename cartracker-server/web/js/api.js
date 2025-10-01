export class API {
  static getToken() {
    const token = localStorage.getItem("token");
    if (token === null) throw new Error("Not logged in");

    return token;
  }

  static async checkToken(token) {
    if (token === "demo") return true;

    const res = await fetch("/api/v1/auth/verify-token", {
      method: "POST",
      headers: {
        "content-type": "application/json",
        authorization: "Bearer " + token,
      },
    });

    return res.ok;
  }

  static async getLocationHistory() {
    const token = this.getToken();
    if (token === "demo") {
      return [
        {
          latitude: 40.72458366021585,
          longitude: -73.99363241032282,
          accuracy: 40,
          createdAt: Date.now() - 10 * 1000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.725461991524405,
          longitude: -73.99629565510395,
          accuracy: 20,
          createdAt: 1759283807000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.72643917170693,
          longitude: -73.99877567882542,
          accuracy: 30,
          createdAt: 1759283707000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.72804853617002,
          longitude: -73.99889527298869,
          accuracy: 50,
          createdAt: 1759283607000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.72933070045135,
          longitude: -73.99784191656005,
          accuracy: 5,
          createdAt: 1759283507000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.73046479664964,
          longitude: -74.00015188656924,
          accuracy: 10,
          createdAt: 1759283407000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.73108169347352,
          longitude: -74.00143343485345,
          accuracy: 10,
          createdAt: 1759283307000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.732897225012294,
          longitude: -74.0001314792564,
          accuracy: 10,
          createdAt: 1759283207000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.731343760295,
          longitude: -73.99683370807853,
          accuracy: 10,
          createdAt: 1759283107000,
          userAgent: "cartracker/1.0",
        },
        {
          latitude: 40.73103441990859,
          longitude: -73.99723624199034,
          accuracy: 30,
          createdAt: 1759283007000,
          userAgent: "cartracker/1.0",
        },
      ];
    }

    const res = await fetch("/api/v1/location/history", {
      headers: {
        "content-type": "application/json",
        authorization: "Bearer " + token,
      },
    });

    return (await res.json()).results;
  }

  static async updateSettings(interval) {
    const token = this.getToken();
    if (token === "demo") {
      throw new Error("You cannot update settings in demo mode");
    }
  }

  static async logout() {
    localStorage.removeItem("token");
  }
}
