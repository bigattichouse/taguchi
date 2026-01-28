/**
 * Example Node.js Usage of Taguchi Array Tool
 * Demonstrates various ways to use the Taguchi Array Tool from JavaScript
 */

const { Taguchi } = require('./taguchi_binding');
const fs = require('fs');
const path = require('path');

function exampleListArrays() {
  console.log('=== Example 1: List Available Arrays ===');
  
  try {
    const arrays = Taguchi.listArrays();
    console.log('Available orthogonal arrays:');
    arrays.forEach(array => console.log(`  ${array}`));
  } catch (error) {
    console.error('Error listing arrays:', error.message);
  }
  console.log();
}

function exampleBasicWorkflow() {
  console.log('=== Example 2: Basic Workflow ===');
  
  // Define an experiment as a string (simulating .tgu file content)
  const tguContent = `factors:
  cache_size: 64M, 128M, 256M
  threads: 2, 4, 8
array: L9`;

  try {
    console.log('Parsing experiment definition...');
    const defHandle = Taguchi.parseDefinition(tguContent);
    console.log('Successfully parsed experiment definition');

    console.log('Validating experiment definition...');
    if (Taguchi.validateDefinition(defHandle)) {
      console.log('Experiment definition is valid!');
    } else {
      console.log('Experiment definition validation failed');
    }

    console.log('Generating runs...');
    const runs = Taguchi.generateRuns(defHandle);
    console.log(`Generated ${runs.length} experiment runs`);

    // Display first few runs
    console.log('\\nGenerated runs:');
    runs.slice(0, 5).forEach(run => {
      console.log(`  Run ${run.id}`);
    });
    
    if (runs.length > 5) {
      console.log(`  ... and ${runs.length - 5} more runs`);
    }

    // Cleanup
    Taguchi.freeDefinition(defHandle);
    console.log('Successfully cleaned up resources');
  } catch (error) {
    console.error('Error in basic workflow:', error.message);
  }
  console.log();
}

function exampleErrorHandling() {
  console.log('=== Example 3: Error Handling ===');
  
  // Test with invalid content
  try {
    const invalidContent = `invalid content`;
    console.log('Attempting to parse invalid content...');
    Taguchi.parseDefinition(invalidContent);
    console.log('Unexpectedly succeeded parsing invalid content');
  } catch (error) {
    console.log('Correctly caught error when parsing invalid content:');
    console.log(`  Error message: ${error.message}`);
  }
  console.log();
}

async function exampleAsyncWorkflow() {
  console.log('=== Example 4: Async Simulation ===');
  
  console.log('Simulating asynchronous experiment run execution...');
  
  // This simulates an async workflow where you might run each experiment
  // in sequence or in parallel
  const tguContent = `factors:
  algorithm_version: v1, v2, v3
  optimization_level: low, medium, high
array: L9`;

  try {
    const defHandle = Taguchi.parseDefinition(tguContent);
    const runs = Taguchi.generateRuns(defHandle);
    
    console.log(`Generated ${runs.length} experiment configurations`);
    
    // Simulate running each configuration asynchronously
    for (let i = 0; i < Math.min(3, runs.length); i++) {
      const run = runs[i];
      console.log(`Running experiment configuration ${run.id}...`);
      
      // Simulate async processing (in real scenario, this could be
      // calling an external process or service)
      await new Promise(resolve => setTimeout(resolve, 100));
      console.log(`  Finished run ${run.id}`);
    }
    
    if (runs.length > 3) {
      console.log(`  ... and ${runs.length - 3} more runs`);
    }
    
    Taguchi.freeDefinition(defHandle);
  } catch (error) {
    console.error('Error in async workflow:', error.message);
  }
  console.log();
}

// Run all examples
async function runExamples() {
  console.log('Taguchi Array Tool - Node.js Examples\\n');
  
  exampleListArrays();
  exampleBasicWorkflow();
  exampleErrorHandling();
  await exampleAsyncWorkflow();
  
  console.log('All Node.js examples completed!');
}

// Run examples when the script is executed directly
if (require.main === module) {
  runExamples().catch(console.error);
}

module.exports = { Taguchi, runExamples };