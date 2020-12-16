import {
    DiagnosticSeverity,
} from 'vscode-languageserver';

export interface Issue {
	severity: DiagnosticSeverity,
	pattern: RegExp,
	message: string,
	start: number
}

export const issues: Issue[] = [
	{ severity: DiagnosticSeverity.Warning, pattern: /\/\/[^\n]*(TO ?DO)/ig, message: 'needs to be worked on', start: 0},
	{ severity: DiagnosticSeverity.Warning, pattern: /(?:class)\s+(\b[a-z][\S]*\b)\s*\{/g, message: 'class identifiers should be UpperCamelCase', start: 5}
];