// Test data for the packages collection
// Run: mongosh package-service data.js

db = db.getSiblingDB("package-service");

db.packages.deleteMany({});

const now = new Date();
const msPerDay = 24 * 60 * 60 * 1000;

db.packages.insertMany([
  // User 1 packages — books and small items
  {
    user_id: NumberLong(1),
    weight: 1.2,
    dimensions: { length: 30.0, width: 22.0, height: 5.0 },
    description: "Books collection",
    created_at: new Date(now - 10 * msPerDay)
  },
  {
    user_id: NumberLong(1),
    weight: 0.35,
    dimensions: { length: 15.0, width: 10.0, height: 3.0 },
    description: "Small electronics",
    created_at: new Date(now - 7 * msPerDay)
  },
  {
    user_id: NumberLong(1),
    weight: 5.8,
    dimensions: { length: 60.0, width: 40.0, height: 30.0 },
    description: "Winter clothes",
    created_at: new Date(now - 3 * msPerDay)
  },

  // User 2 packages — heavy items
  {
    user_id: NumberLong(2),
    weight: 12.5,
    dimensions: { length: 50.0, width: 50.0, height: 50.0 },
    description: "Power tools set",
    created_at: new Date(now - 14 * msPerDay)
  },
  {
    user_id: NumberLong(2),
    weight: 0.1,
    dimensions: { length: 8.0, width: 5.0, height: 1.0 },
    description: "Documents envelope",
    created_at: new Date(now - 5 * msPerDay)
  },

  // User 3 packages
  {
    user_id: NumberLong(3),
    weight: 3.3,
    dimensions: { length: 40.0, width: 30.0, height: 20.0 },
    description: "Kitchen appliance",
    created_at: new Date(now - 20 * msPerDay)
  },
  {
    user_id: NumberLong(3),
    weight: 25.0,
    dimensions: { length: 120.0, width: 80.0, height: 60.0 },
    description: "Bicycle parts",
    created_at: new Date(now - 1 * msPerDay)
  },

  // User 4 packages
  {
    user_id: NumberLong(4),
    weight: 0.5,
    dimensions: { length: 20.0, width: 15.0, height: 10.0 },
    description: "Gift box",
    created_at: new Date(now - 2 * msPerDay)
  },
  {
    user_id: NumberLong(4),
    weight: 7.2,
    dimensions: { length: 70.0, width: 35.0, height: 25.0 },
    description: "Sports equipment",
    created_at: new Date(now - 8 * msPerDay)
  },

  // User 5 packages
  {
    user_id: NumberLong(5),
    weight: 2.1,
    dimensions: { length: 25.0, width: 20.0, height: 15.0 },
    description: "Fragile glassware",
    created_at: new Date(now - 4 * msPerDay)
  },
  {
    user_id: NumberLong(5),
    weight: 49.9,
    dimensions: { length: 200.0, width: 100.0, height: 80.0 },
    description: "Furniture piece",
    created_at: new Date(now - 6 * msPerDay)
  },

  // User 6 — recent packages
  {
    user_id: NumberLong(6),
    weight: 1.0,
    dimensions: { length: 10.0, width: 10.0, height: 10.0 },
    description: "Cube package",
    created_at: new Date(now - 1 * msPerDay)
  }
]);

print("Inserted " + db.packages.countDocuments() + " packages.");
