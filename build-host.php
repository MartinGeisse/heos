#!/usr/bin/env php
<?php

function buildFileForTarget($inputPath, $outputPath, $extension) {
    if ($extension == 'S') {
        return;
    }
    $allowedExtensions = array('c', 'cpp');
    if (!in_array($extension, $allowedExtensions)) {
        die('unknown input file extension: ' . $extension);
    }
    $cppFlags = ($extension == '.cpp' ? ' -fno-rtti ' : '');
    $baseCommand = 'gcc -fno-exceptions ' . $cppFlags . ' -Wall -Wextra -O3 -fno-tree-loop-distribute-patterns';
    system($baseCommand . ' -c  -o ' . $outputPath . ' ' . $inputPath);
}

function linkFiles() {
    global $objectFiles;
    $objectFilesList = implode(' ', $objectFiles);
    system('ld -Map=out-host/program.map -o out-host/program.elf ' . $objectFilesList);
}

require('buildscript/common.php');

//
// --------------------------------------------------------------------------------------------------------------------
//

system('rm -rf out-host');
system('mkdir out-host');

buildDirectory('src', 'out-host');
linkFiles();
