<?php

$objectFiles = array();

function prepareOutputFolder() {
    system('rm -rf out-' . TARGET);
    system('mkdir out-' . TARGET);
}

function buildFile($inputPath, $outputPath) {
    $dotPosition = strrpos($inputPath, '.');
    if ($dotPosition === FALSE) {
        die('no dot in input filename');
    }
    $extension = substr($inputPath, $dotPosition + 1);
    if ($extension == 'h') {
        return;
    }
    if (buildFileForTarget($inputPath, $outputPath, $extension)) {
        global $objectFiles;
        $objectFiles[] = $outputPath;
    }
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
            $dashPosition = strrpos($filename, '-');
            if ($dashPosition === FALSE || substr($filename, $dashPosition + 1) == TARGET) {
                $inputFolder = $inputPath . '/' . $filename;
                $outputFolder = $outputPath . '/' . $filename;
                system('mkdir ' . $outputFolder);
                buildDirectory($inputFolder, $outputFolder);
            }
        } else {
            $baseName = substr($filename, 0, $dotPosition);
            $inputFile = $inputPath . '/' . $filename;
            $outputFile = $outputPath . '/' . $baseName . '.o';
            buildFile($inputFile, $outputFile);
        }
    }
}
