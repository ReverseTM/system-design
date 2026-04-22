// Schema validation for the packages collection
// Run: mongosh package-service validation.js

db = db.getSiblingDB("package-service");

db.createCollection("packages", {
  validator: {
    $jsonSchema: {
      bsonType: "object",
      title: "Package Validation",
      required: ["user_id", "weight", "dimensions", "description", "created_at"],
      additionalProperties: false,
      properties: {
        _id: {
          bsonType: "objectId"
        },
        user_id: {
          bsonType: "long",
          minimum: 1,
          description: "must be a positive integer (user reference)"
        },
        weight: {
          bsonType: "double",
          minimum: 0.01,
          maximum: 1000.0,
          description: "weight in kg, must be > 0"
        },
        dimensions: {
          bsonType: "object",
          required: ["length", "width", "height"],
          additionalProperties: false,
          properties: {
            length: {
              bsonType: "double",
              minimum: 0.01,
              maximum: 500.0,
              description: "length in cm, must be > 0"
            },
            width: {
              bsonType: "double",
              minimum: 0.01,
              maximum: 500.0,
              description: "width in cm, must be > 0"
            },
            height: {
              bsonType: "double",
              minimum: 0.01,
              maximum: 500.0,
              description: "height in cm, must be > 0"
            }
          }
        },
        description: {
          bsonType: "string",
          maxLength: 255,
          description: "package description, max 255 chars"
        },
        created_at: {
          bsonType: "date",
          description: "creation timestamp"
        }
      }
    }
  },
  validationAction: "error",
  validationLevel: "strict"
});

db.packages.createIndex(
  { user_id: 1, created_at: -1 },
  { name: "idx_packages_user_created" }
);

print("Collection 'packages' created with schema validation.");
print("Index 'idx_packages_user_created' created.");

// Test: insert invalid document (weight = -1) to verify validation works
print("\n--- Validation test: inserting document with weight=-1 ---");
try {
  db.packages.insertOne({
    user_id: NumberLong(1),
    weight: -1.0,
    dimensions: { length: 10.0, width: 5.0, height: 3.0 },
    description: "invalid weight test",
    created_at: new Date()
  });
  print("ERROR: validation did not reject invalid document!");
} catch (e) {
  print("OK: validation rejected invalid document: " + e.message);
}

// Test: insert invalid document (missing required field)
print("\n--- Validation test: inserting document without user_id ---");
try {
  db.packages.insertOne({
    weight: 1.5,
    dimensions: { length: 10.0, width: 5.0, height: 3.0 },
    description: "missing user_id test",
    created_at: new Date()
  });
  print("ERROR: validation did not reject document without user_id!");
} catch (e) {
  print("OK: validation rejected document without user_id: " + e.message);
}

print("\nValidation setup complete.");
