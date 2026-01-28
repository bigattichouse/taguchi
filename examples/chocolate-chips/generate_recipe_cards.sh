#!/bin/bash
# generate_recipe_cards.sh
# Generates printable recipe cards for manual cookie experiment execution

if [ ! -f "cookie_experiment.tgu" ]; then
    echo "Creating sample cookie experiment definition..."
    cat > cookie_experiment.tgu << 'EOF'
factors:
  butter: 1/2 cup, 3/4 cup, 1 cup
  sugar_ratio: 1:1 brown:white, 2:1 brown:white, 3:1 brown:white
  flour_type: all_purpose, bread_flour, cake_flour
  eggs: 1, 2, 3
  chips: 1/2 cup, 3/4 cup, 1 cup
  baking_temp: 325F, 350F, 375F
  baking_time: 8min, 10min, 12min
array: L27
EOF
fi

EXPERIMENT_FILE="cookie_experiment.tgu"
OUTPUT_FILE="recipe_cards.txt"

echo "CHOCOLATE CHIP COOKIE EXPERIMENT RECIPE CARDS" > "$OUTPUT_FILE"
echo "===========================================" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"
echo "Generated: $(date)" >> "$OUTPUT_FILE"
echo "Total runs: 27" >> "$OUTPUT_FILE"
echo "" >> "$OUTPUT_FILE"

# Generate runs using our taguchi tool
echo "Generating experimental runs..."
./taguchi generate "$EXPERIMENT_FILE" > temp_runs.txt 2>/dev/null

# Parse and format the runs
run_counter=0
while IFS= read -r line; do
    if [[ $line =~ ^Run[[:space:]]+([0-9]+):[[:space:]]+(.*)$ ]]; then
        run_id="${BASH_REMATCH[1]}"
        factors="${BASH_REMATCH[2]}"
        
        echo "RECIPE CARD $run_id" >> "$OUTPUT_FILE"
        echo "=============" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
        
        # Format the factors nicely
        echo "INGREDIENT MEASUREMENTS:" >> "$OUTPUT_FILE"
        IFS=', ' read -ra factor_array <<< "$factors"
        for factor in "${factor_array[@]}"; do
            # Split factor name and value (format: name=value)
            if [[ $factor == *"="* ]]; then
                name="${factor%%=*}"
                value="${factor#*=}"
                
                # Convert factor names to more readable form
                case "$name" in
                    "butter") echo "  - Butter: $value" ;;
                    "sugar_ratio") echo "  - Sugar Ratio (brown:white): $value" ;;
                    "flour_type") echo "  - Flour Type: $value" ;;
                    "eggs") echo "  - Eggs: $value" ;;
                    "chips") echo "  - Chocolate Chips: $value" ;;
                    "baking_temp") echo "  - Oven Temperature: $value" ;;
                    "baking_time") echo "  - Baking Time: $value" ;;
                    *) echo "  - $name: $value" ;;
                esac
            fi
        done
        echo "" >> "$OUTPUT_FILE"
        
        echo "INSTRUCTIONS:" >> "$OUTPUT_FILE"
        echo "  1. Preheat oven to specified temperature" >> "$OUTPUT_FILE"
        echo "  2. Cream butter and sugars until light and fluffy" >> "$OUTPUT_FILE"
        echo "  3. Add eggs one at a time, then vanilla" >> "$OUTPUT_FILE"
        echo "  4. In separate bowl, whisk flour and baking soda" >> "$OUTPUT_FILE"
        echo "  5. Gradually add dry ingredients to wet ingredients" >> "$OUTPUT_FILE"
        echo "  6. Fold in chocolate chips" >> "$OUTPUT_FILE"
        echo "  7. Scoop dough onto parchment-lined baking sheet" >> "$OUTPUT_FILE"
        echo "  8. Bake for specified time (until edges are golden)" >> "$OUTPUT_FILE"
        echo "  9. Cool on baking sheet for 5 minutes, then transfer to wire rack" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
        
        echo "RATING SHEET:" >> "$OUTPUT_FILE"
        echo "  Taste Score (1-10): _______" >> "$OUTPUT_FILE"
        echo "  Texture Score (1-10): _______" >> "$OUTPUT_FILE"
        echo "  Yield Count: _______" >> "$OUTPUT_FILE"
        echo "  Appearance Score (1-10): _______" >> "$OUTPUT_FILE"
        echo "  Notes: _________________________" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
        
        echo "----------------------------------------" >> "$OUTPUT_FILE"
        echo "" >> "$OUTPUT_FILE"
        
        ((run_counter++))
    fi
done < temp_runs.txt

rm temp_runs.txt

echo "Recipe cards generated: $OUTPUT_FILE"
echo "Print these cards and use them to execute your cookie experiments systematically."
echo "Complete all 27 runs then analyze results with:"
echo "  ./taguchi analyze cookie_experiment.tgu results.csv"
echo ""
echo "Results should be in CSV format like:"
echo "run_id,taste_score,texture_score,yield_count,appearance_score"