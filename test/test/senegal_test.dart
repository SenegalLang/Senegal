import 'dart:io';

import 'package:test/test.dart' as test;
import 'package:test_process/test_process.dart';

class ExpectationsParser {
  ExpectationsParser(this.file) {
    _parseExpectations();
  }

  final File file;

  String workingDir = '.';

  final _outputs = <String>[];
  int _matchedOutputs = 0;

  final _errors = <String>[];
  int _matchedErrors = 0;

  void _parseExpectations() {
    final content = file.readAsStringSync();
    final lines = content.split(RegExp(r'\n|(\r\n)'));
    // repeat variables
    bool isRepeat = false;
    List<String> repeatOutputs = [];
    int count = 0;

    if (lines.first.startsWith('// root'))
      workingDir = '../';

    lines.forEach((line) {
      final commentStart = line.indexOf('//');
      if (commentStart == -1)
        return;
      // skip the two forward slashes
      final comment = line.substring(commentStart+2).trimLeft();
      if (comment.startsWith('output ')) {
        if (isRepeat) {
          repeatOutputs.add(comment.substring('output '.length));
        } else {
          _outputs.add(comment.substring('output '.length));
        }
      } else if (comment.startsWith('error ')) {
        _errors.add(comment.substring('error '.length));
      } else if (comment.startsWith('ignore')) {
        if (isRepeat) {
          repeatOutputs.add(null);
        } else {
          _outputs.add(null);
        }
      } else if (comment.startsWith('repeat ')) {
        count = int.parse(comment.substring('repeat '.length));
        isRepeat = true;
      } else if (comment.startsWith('endrepeat')) {
        for (int i = 0; i < count; ++i)
          _outputs.addAll(repeatOutputs);
        isRepeat = false;
        repeatOutputs = [];
        count = 0;
      }
    });
    if (isRepeat) {
      print("Warning: repeat directive didn't find a matching endrepeat.");
    }
  }

  bool doesExpectOutput() => _outputs.isNotEmpty && _matchedOutputs != _outputs.length;

  String nextOutput() {
    assert(_matchedOutputs < _outputs.length);
    return _outputs[_matchedOutputs++];
  }

  bool doesExpectError() => _errors.isNotEmpty && _matchedErrors != _errors.length;

  String nextError() {
    assert(_matchedErrors < _errors.length);
    return _errors[_matchedErrors++];
  }

  String getError() => _matchedErrors == _errors.length ? null : _errors[_matchedErrors];
}

void main() {

  test.group('All test programs run with expected results', () {
    final Directory directory = Directory('./test_programs/');
    directory.listSync().forEach((file) {
      test.test(file.path, () async {
        await _testBirbScriptWithExpectations(file);
      });
    });
  });
}

Future _testBirbScriptWithExpectations(FileSystemEntity file) async {
  final expectations = ExpectationsParser(file);

  final process = await TestProcess.start('./senegal', ['.${expectations.workingDir == '../' ? '/test/' : ''}${file.path.replaceFirst('.', '')}'], workingDirectory: expectations.workingDir, runInShell: true);

  final shouldFailOnError = !expectations.doesExpectError();
  await for (final line in process.stderrStream()) {
    if (shouldFailOnError) {
      test.fail('Unexpected error');
    }
    if (expectations.doesExpectError() && line.contains(expectations.getError())) {
      expectations.nextError();
    }
  }
  if (expectations.doesExpectError()) {
    test.fail('expected ${expectations.nextError()} error next');
  }

  final ignored = <String>[];
  await for (final line in process.stdoutStream()) {
    if (expectations.doesExpectOutput()) {
      final expected = expectations.nextOutput();
      if (expected == null) {
        ignored.add(line);
      } else {
        test.expect(line, test.equals(expected));
      }
    } else {
      test.fail('Too many outputs: unexpected $line');
    }
  }
  if (expectations.doesExpectOutput()) {
    test.fail('Too few outputs: expected ${expectations.nextOutput()} next');
  }
  await process.shouldExit();

  if (ignored.isNotEmpty) {
    print('Warning: ignoring output lines:');
    print(ignored);
  }
}
