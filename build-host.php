#!/usr/bin/env php
<?php

define('TARGET', 'host');

function buildFileForTarget($inputPath, $outputPath, $extension) {
    if ($extension == 'S') {
        return FALSE;
    }
    $allowedExtensions = array('c', 'cpp');
    if (!in_array($extension, $allowedExtensions)) {
        die('unknown input file extension: ' . $extension);
    }
    $cppFlags = ($extension == '.cpp' ? ' -fno-rtti ' : '');
    $baseCommand = 'gcc -DTARGET_HOST -fno-exceptions ' . $cppFlags .
        ' -Wall -Wextra -Werror -O3 -fno-tree-loop-distribute-patterns';
    system($baseCommand . ' -c  -o ' . $outputPath . ' ' . $inputPath);
    return TRUE;
}

function linkFiles() {
    global $objectFiles;
    $objectFilesList = implode(' ', $objectFiles);
    system('gcc -lc -Wl,-Map=out-host/program.map -o out-host/program.elf ' . $objectFilesList);
}

require('buildscript/common.php');

//
// --------------------------------------------------------------------------------------------------------------------
//

prepareOutputFolder();
buildDirectory('src', 'out-host');
linkFiles();
