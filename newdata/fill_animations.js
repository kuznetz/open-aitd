// Fills the "animations" field of each character in newdata/characters.json
// with a flat, sorted, unique (union) list of animation ids taken from all
// models of that character.
//
// Sources:
//   - newdata/names/models.json   -> { "<modelId>": "<MODEL_NAME>", ... }
//   - newdata/animation_links.json -> [ { "modelId": <id>, "anims": [<id>, ...] }, ... ]
//   - newdata/characters.json      -> [ { "name": ..., "models": ["NAME", ...], "animations": [] }, ... ]

const fs = require('fs').promises;
const path = require('path');

async function readJson(filePath) {
  const raw = await fs.readFile(filePath, 'utf8');
  return JSON.parse(raw);
}

async function main() {
  const namesPath = path.join(__dirname, 'names', 'models.json');
  const linksPath = path.join(__dirname, 'animation_links.json');
  const charsPath = path.join(__dirname, 'characters.json');

  const names = await readJson(namesPath); // { "<id>": "NAME", ... }
  const links = await readJson(linksPath); // [ { modelId, anims }, ... ]
  const chars = await readJson(charsPath); // [ { name, models, animations }, ... ]

  // name -> modelId
  const nameToId = new Map();
  for (const [id, name] of Object.entries(names)) {
    nameToId.set(name, Number(id));
  }

  // modelId -> anims
  const idToAnims = new Map();
  for (const link of links) {
    const id = Number(link.modelId);
    idToAnims.set(id, (link.anims || []).map(Number));
  }

  for (const ch of chars) {
    const union = new Set();

    for (const modelName of ch.models || []) {
      const id = nameToId.get(modelName);
      if (id === undefined) {
        console.warn(`[warn] model "${modelName}" not found in models.json (character "${ch.name}")`);
        continue;
      }

      const anims = idToAnims.get(id);
      if (!anims) {
        console.warn(`[warn] no animation link for model "${modelName}" (id ${id})`);
        continue;
      }

      for (const a of anims) union.add(a);
    }

    ch.animations = [...union].sort((a, b) => a - b);
  }

  await fs.writeFile(charsPath, JSON.stringify(chars, null, 2) + '\n', 'utf8');
  console.log(`done. characters: ${chars.length}`);
}

main().catch((err) => {
  console.error('error:', err.message);
  process.exit(1);
});
