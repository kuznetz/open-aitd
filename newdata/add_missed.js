const fs = require('fs').promises;
const path = require('path');

function addMissingAnimations(data, missing) {
    const result = data.map(item => ({
        ...item,
        anims: [...item.anims]
    }));

    for (const miss of missing) {
        const existing = result.find(item => item.modelId === miss.model);
        if (existing) {
            const newAnims = miss.anims.filter(a => !existing.anims.includes(a));
            existing.anims.push(...newAnims);
        } else {
            result.push({
                modelId: miss.model,
                anims: [...miss.anims]
            });
        }
    }
    return result;
}

async function main() {
    try {
        const dataFilePath = path.join(__dirname, 'animation_links.json');
        const missingFilePath = path.join(__dirname, '../data/missed_anims.json');
        const outputFilePath = path.join(__dirname, 'animation_links.json');

        const dataRaw = await fs.readFile(dataFilePath, 'utf8');
        const data = JSON.parse(dataRaw);
        const missingRaw = await fs.readFile(missingFilePath, 'utf8');
        const missing = JSON.parse(missingRaw);

        const updatedData = addMissingAnimations(data, missing);

        await fs.writeFile(outputFilePath, JSON.stringify(updatedData, null, 2), 'utf8');

        console.log('done.');
    } catch (err) {
        console.error('error:', err.message);
        process.exit(1);
    }
}

main();