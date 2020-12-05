import {
    createConnection,
    TextDocuments,
    Diagnostic,
    DiagnosticSeverity,
    ProposedFeatures,
    InitializeParams,
    DidChangeConfigurationNotification,
    CompletionItem,
    CompletionItemKind,
    TextDocumentPositionParams,
    TextDocumentSyncKind,
    InitializeResult
} from 'vscode-languageserver';

import {
    TextDocument
} from 'vscode-languageserver-textdocument';
import { Issue, issues } from './issues';

let connection = createConnection(ProposedFeatures.all);

let documents: TextDocuments<TextDocument> = new TextDocuments(TextDocument);
let hasConfigurationCapability: boolean = false;
let hasWorkspaceFolderCapability: boolean = false;

function handleCompletionItem(data: number, item: (CompletionItem | undefined)) {
}

connection.onInitialize((params: InitializeParams) => {
    let capabilities = params.capabilities;

    hasConfigurationCapability = !!(
        capabilities.workspace && !!capabilities.workspace.configuration
    );
    hasWorkspaceFolderCapability = !!(
        capabilities.workspace && !!capabilities.workspace.workspaceFolders
    );

    const result: InitializeResult = {
        capabilities: {
            textDocumentSync: TextDocumentSyncKind.Incremental,
            completionProvider: {
                resolveProvider: true
            }
        }
    };
    if (hasWorkspaceFolderCapability) {
        result.capabilities.workspace = {
            workspaceFolders: {
                supported: true
            }
        };
    }
    return result;
});

connection.onInitialized(() => {
    if (hasConfigurationCapability) {
        connection.client.register(DidChangeConfigurationNotification.type, undefined);
    }
    if (hasWorkspaceFolderCapability) {
        connection.workspace.onDidChangeWorkspaceFolders(_event => {
            
        });
    }
});


connection.onDidChangeConfiguration(() => {
    documents.all().forEach(validateTextDocument);
});

documents.onDidChangeContent(change => {
    validateTextDocument(change.document);
});

async function validateTextDocument(textDocument: TextDocument): Promise<void> {
    let diagnostics: Diagnostic[] = [...validatePatterns(textDocument)];

    connection.sendDiagnostics({ uri: textDocument.uri, diagnostics });
}

function validatePatterns (textDocument: TextDocument): Diagnostic[] {
    let diagnosticResults: Diagnostic[] = [];

    issues.forEach(issue => {
        diagnosticResults = diagnosticResults.concat(validateWithPattern(textDocument, issue));
    });

    return diagnosticResults;
}

function validateWithPattern (textDocument: TextDocument, issue: Issue): Diagnostic[] {
    const severity: DiagnosticSeverity = issue.severity;
    const pattern: RegExp = issue.pattern;
    const message: string = issue.message;
    const text = textDocument.getText();

    const diagnosticResults: Diagnostic[] = [];
    let m: RegExpExecArray | null;

    while (m = pattern.exec(text)) {
        const startPosition: number = m.index + m[0].indexOf(m[1], issue.start);

        diagnosticResults.push({
            severity,
            range: {
                start: textDocument.positionAt(startPosition),
                end: textDocument.positionAt(startPosition + m[1].length)
            },
            message: `${m[1]}: ${message}.`,
            source: textDocument.uri.substring(textDocument.uri.lastIndexOf("/") + 1)
        });
    }

    return diagnosticResults;
}


connection.onDefinition

connection.onDidChangeWatchedFiles(_change => {
    
});

documents.listen(connection);

connection.listen();