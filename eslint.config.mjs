import { FlatCompat } from "@eslint/eslintrc";
import js from "@eslint/js";
import typescriptEslint from "@typescript-eslint/eslint-plugin";
import tsParser from "@typescript-eslint/parser";
import neverthrow from "eslint-plugin-neverthrow";
import globals from "globals";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const compat = new FlatCompat({
    baseDirectory: __dirname,
    recommendedConfig: js.configs.recommended,
    allConfig: js.configs.all,
});

export default [
    ...compat.extends(
        "eslint:recommended",
        "plugin:@typescript-eslint/recommended",
    ),
    {
        plugins: {
            "@typescript-eslint": typescriptEslint,
            neverthrow,
        },

        languageOptions: {
            globals: {
                ...globals.node,
            },
            parser: tsParser,
            ecmaVersion: "latest",
            sourceType: "module",
        },
    },
    {
        files: ["**/.eslintrc.{js,cjs}"],

        languageOptions: {
            globals: {
                ...globals.node,
            },

            ecmaVersion: 5,
            sourceType: "commonjs",
        },
    },
];
