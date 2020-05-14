<?php

$objectFiles = array();

function buildFile($inputPath, $outputPath) {
    global $objectFiles;
    $objectFiles[] = $outputPath;

    $dotPosition = strrpos($inputPath, '.');
    if ($dotPosition === FALSE) {
        die('no dot in input filename');
    }
    $extension = substr($inputPath, $dotPosition + 1);
    if ($extension == 'h') {
        return;
    }
    buildFileForTarget($inputPath, $outputPath, $extension);
}

function buildDirectory($inputPath, $outputPath) {
    $filenames = scandir($inputPath);
    sort($filenames);
    foreach ($filenames as $filename) {
        if ($filename == '.' || $filename == '..') {
            continue;
        }
        $dotPosition = strrpos($filename, '.');
        if ($dotPosition === FALSE) {
            $inputFolder = $inputPath . '/' . $filename;
            $outputFolder = $outputPath . '/' . $filename;
            system('mkdir ' . $outputFolder);
            buildDirectory($inputFolder, $outputFolder);
        } else {
            $baseName = substr($filename, 0, $dotPosition);
            $inputFile = $inputPath . '/' . $filename;
            $outputFile = $outputPath . '/' . $baseName . '.o';
            buildFile($inputFile, $outputFile);
        }
    }
}
