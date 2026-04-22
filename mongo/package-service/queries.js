// MongoDB CRUD queries and aggregation for package-service
// Run: mongosh package-service queries.js

db = db.getSiblingDB("package-service");

print("=== CREATE ===");

// insertOne — create a new package
const insertResult = db.packages.insertOne({
  user_id: NumberLong(7),
  weight: 2.5,
  dimensions: { length: 30.0, width: 20.0, height: 15.0 },
  description: "New test package",
  created_at: new Date()
});
print("Inserted _id: " + insertResult.insertedId);

print("\n=== READ ===");

// Find by exact user_id ($eq)
print("\n-- All packages for user_id=1 ($eq) --");
db.packages.find(
  { user_id: { $eq: NumberLong(1) } },
  { _id: 1, weight: 1, description: 1 }
).sort({ created_at: -1 }).forEach(printjson);

// Find packages heavier than 5 kg ($gt)
print("\n-- Packages with weight > 5 ($gt) --");
db.packages.find(
  { weight: { $gt: 5.0 } },
  { user_id: 1, weight: 1, description: 1 }
).forEach(printjson);

// Find packages lighter than 1 kg ($lt)
print("\n-- Packages with weight < 1 ($lt) --");
db.packages.find(
  { weight: { $lt: 1.0 } },
  { user_id: 1, weight: 1, description: 1 }
).forEach(printjson);

// Find packages for user_id=2 AND weight > 5 ($and)
print("\n-- Packages for user_id=2 AND weight > 5 ($and) --");
db.packages.find({
  $and: [
    { user_id: NumberLong(2) },
    { weight: { $gt: 5.0 } }
  ]
}).forEach(printjson);

// Find packages for user_id in [1, 3, 5] ($in)
print("\n-- Packages for users [1, 3, 5] ($in) --");
db.packages.find(
  { user_id: { $in: [NumberLong(1), NumberLong(3), NumberLong(5)] } },
  { user_id: 1, weight: 1, description: 1 }
).sort({ user_id: 1, created_at: -1 }).forEach(printjson);

// Find packages where weight < 0.5 OR weight > 20 ($or)
print("\n-- Packages where weight < 0.5 OR weight > 20 ($or) --");
db.packages.find({
  $or: [
    { weight: { $lt: 0.5 } },
    { weight: { $gt: 20.0 } }
  ]
}, { user_id: 1, weight: 1, description: 1 }).forEach(printjson);

// Find by nested field: dimensions.length > 100
print("\n-- Packages with dimensions.length > 100 --");
db.packages.find(
  { "dimensions.length": { $gt: 100.0 } },
  { user_id: 1, weight: 1, dimensions: 1, description: 1 }
).forEach(printjson);

print("\n=== UPDATE ===");

// updateOne — update description and weight ($set)
const updateResult = db.packages.updateOne(
  { user_id: NumberLong(1), description: "Books collection" },
  {
    $set: {
      description: "Books collection (updated)",
      weight: 1.5
    }
  }
);
print("Matched: " + updateResult.matchedCount + ", Modified: " + updateResult.modifiedCount);

// updateMany — add suffix to all packages for user_id=3
const updateManyResult = db.packages.updateMany(
  { user_id: NumberLong(3) },
  { $set: { "dimensions.height": 25.0 } }
);
print("updateMany matched: " + updateManyResult.matchedCount + ", modified: " + updateManyResult.modifiedCount);

print("\n=== DELETE ===");

// deleteOne — delete the package we inserted above
const deleteResult = db.packages.deleteOne({
  user_id: NumberLong(7),
  description: "New test package"
});
print("Deleted: " + deleteResult.deletedCount);

print("\n=== AGGREGATION PIPELINE ===");

// Pipeline: for each user — count packages, total weight, avg weight
// Filter only users with more than 1 package, sort by total weight desc
print("\n-- User package statistics --");
db.packages.aggregate([
  // Stage 1: match packages created in the last 30 days
  {
    $match: {
      created_at: { $gte: new Date(Date.now() - 30 * 24 * 60 * 60 * 1000) }
    }
  },
  // Stage 2: group by user_id
  {
    $group: {
      _id: "$user_id",
      package_count: { $sum: 1 },
      total_weight: { $sum: "$weight" },
      avg_weight: { $avg: "$weight" },
      max_weight: { $max: "$weight" }
    }
  },
  // Stage 3: filter users with more than 1 package
  {
    $match: {
      package_count: { $gt: 1 }
    }
  },
  // Stage 4: reshape output
  {
    $project: {
      _id: 0,
      user_id: "$_id",
      package_count: 1,
      total_weight: { $round: ["$total_weight", 2] },
      avg_weight: { $round: ["$avg_weight", 2] },
      max_weight: 1
    }
  },
  // Stage 5: sort by total weight descending
  {
    $sort: { total_weight: -1 }
  }
]).forEach(printjson);

print("\nAll queries executed successfully.");
