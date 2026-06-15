const { MongoClient } = require("mongodb");

const MONGO_URI = process.env.MONGO_URI || "mongodb://mongo:27017";
const MONGO_DB = process.env.MONGO_DB || "dc-monitor";
const EVENTS_TTL_SECONDS = Number(process.env.EVENTS_TTL_SECONDS || 604800); // 7 days

let client = null;
let db = null;

async function connect() {
  client = new MongoClient(MONGO_URI, {
    serverSelectionTimeoutMS: 5000,
    connectTimeoutMS: 5000,
  });
  await client.connect();
  db = client.db(MONGO_DB);

  await db.collection("events").createIndex(
    { createdAt: 1 },
    { expireAfterSeconds: EVENTS_TTL_SECONDS }
  );
  await db.collection("events").createIndex({ serialNumber: 1 });
  await db.collection("devices").createIndex({ lastSeenAt: -1 });

  console.log(`[db] connected to ${MONGO_URI}/${MONGO_DB}`);
  return db;
}

function isConnected() {
  return client && client.topology && client.topology.isConnected();
}

async function loadState() {
  const devices = {};
  const events = [];
  try {
    const deviceDocs = await db.collection("devices").find({}).toArray();
    for (const doc of deviceDocs) {
      const { _id, ...rest } = doc;
      devices[_id] = rest;
    }
    const eventDocs = await db.collection("events")
      .find({})
      .sort({ createdAt: -1 })
      .limit(100)
      .toArray();
    for (const doc of eventDocs) {
      const { _id, createdAt, ...rest } = doc;
      events.push(rest);
    }
    console.log(`[db] loaded ${Object.keys(devices).length} devices, ${events.length} events`);
  } catch (err) {
    console.error(`[db] loadState error: ${err.message}`);
  }
  return { devices, events };
}

async function upsertDevice(deviceId, data) {
  if (!db) return;
  try {
    const { _id, lastSeenAt, ...setData } = data;
    await db.collection("devices").updateOne(
      { _id: deviceId },
      { $set: { ...setData, lastSeenAt: data.lastSeenAt }, $setOnInsert: { _id: deviceId } },
      { upsert: true }
    );
  } catch (err) {
    console.error(`[db] upsertDevice error: ${err.message}`);
  }
}

async function insertEvent(event) {
  if (!db) return;
  try {
    await db.collection("events").insertOne({
      ...event,
      createdAt: new Date(),
    });
  } catch (err) {
    console.error(`[db] insertEvent error: ${err.message}`);
  }
}

async function clearAll() {
  if (!db) return;
  try {
    await db.collection("devices").deleteMany({});
    await db.collection("events").deleteMany({});
    console.log("[db] cleared all data");
  } catch (err) {
    console.error(`[db] clearAll error: ${err.message}`);
  }
}

async function disconnect() {
  if (client) await client.close();
}

module.exports = { connect, isConnected, loadState, upsertDevice, insertEvent, clearAll, disconnect };
