#include <iostream>
#include <string>
#include <cmath>
#include <array>
#include <vector>
#include <fstream>
#include <list>
#include <algorithm>


using namespace std;

// Function declarations
void displayWelcomeMessage();
void promptUsage();
void processData(const std::string& filename, bool isMultipleYears = false, bool folder = false);
void convertMJToKWh(vector<vector<string>>& data, int solarColIndex);
void writeConvertedData(const string& filename, const vector<vector<string>>& data);
void logInteraction(const string& message);
string getDate(int year, int month, int day);
void findMinMaxMean(const vector<vector<string>>& data, int solarColIndex, vector<string>& results);
void bubbleSort(std::vector<double>& vec);
void findMode(const std::vector<double>& exposures);
void findMonthlyStats(const vector<vector<string>>& data, int solarColIndex, vector<string>& results);
void writeResultsToCSV(const string& filename, const vector<string>& results);
void displayPreviousCalculations();
bool isInYearRange(const std::string& year, const std::string& range);


vector<string> history;


int main() {
    displayWelcomeMessage();
    promptUsage();

    char choice;
    std::string dataset;

    while (true) {
        std::cout << "Would you like to open the data from:\n"
            << "1. Avalon (Single Year)\n"
            << "2. Caulfield Racecourse (Single Year)\n"
            << "3. Avalon (All Years)\n"
            << "4. Caulfield Racecourse (All Years)\n"
            << "5. Both locations\n"
            << "6. Enter the name of a .csv file\n"
            << "7. Analyze all files in the data_files folder\n" 
            << "Enter your choice (1, 2, 3, 4, 5, 6, or 7): ";
        std::cin >> choice;
        logInteraction("User choice: " + std::string(1, choice));

        switch (choice) {
        case '1':
            dataset = "Avalon_2023_Data.csv";
            processData(dataset, false, false);
            break;
        case '2':
            dataset = "Caulfeild_Racecourse_2023_Data.csv";
            processData(dataset, false, false);
            break;
        case '3':
            dataset = "Avalon_All_Years_Data.csv";
            processData(dataset, true, false);
            break;
        case '4':
            dataset = "Caulfeild_Racecourse_All_Years_Data.csv";
            processData(dataset, true, false);
            break;
        case '5':
            dataset = "Avalon_2023_Data.csv";
            processData(dataset, false, false);
            dataset = "Caulfeild_Racecourse_2023_Data.csv";
            processData(dataset, false, false);
            break;
        case '6':
            std::cout << "Enter the name of the .csv file: ";
            std::cin.ignore();
            std::getline(std::cin, dataset);
            processData(dataset, true, false);
            break;
        case '7': {
            int fileIndex = 1;
            int fileCount = 0;
            while (true) {
                dataset = "data_files/" + to_string(fileIndex) + ".csv";
                ifstream file(dataset);
                if (!file.is_open()) {
                    break;
                }
                file.close();
                processData(dataset, true, true);
                fileCount++;
                fileIndex++;
            }
            cout << "Total files found and processed: " << fileCount << endl;
            break; 
        }
        default:
            std::cout << "Invalid choice. Please enter 1, 2, 3, 4, 5, 6, or 7.\n";
            continue;
        }

        // Ask user if they want to display previous calculations
        std::cout << "Do you want to display previous calculations? (y/n): ";
        char displayChoice;
        std::cin >> displayChoice;

        if (displayChoice == 'y' || displayChoice == 'Y') {
            displayPreviousCalculations();
        }

        // Ask if the user wants to continue or exit
        std::cout << "Would you like to continue? Enter 'C' to continue or 'E' to exit: ";
        char cont;
        std::cin >> cont;
        logInteraction("User continuation choice: " + std::string(1, cont));

        if (cont == 'E' || cont == 'e') {
            break;
        }
    }

    return 0;
}




// Displays the welcome message
void displayWelcomeMessage() {
    cout << "Turing Moore Engineering Solar Exposure Analyser 1.0\n";
    cout << "Name: Jehan Waduge\n";
    cout << "Student ID: 223192064\n";
    cout << "Date Due: 19/09/2024\n";
    cout << "Desired Level: HD\n";
    logInteraction("Displayed Welcome Message");
}

// Tells user on how to use the software
void promptUsage() {
    cout << "Welcome to the Solar Exposure Analyser.\n";
    cout << "This program allows you to select solar exposure data and convert it to kWh/m^2.\n";
    logInteraction("Displayed usage instructions");
}


// Function to check if a year is within a range
bool isInYearRange(const std::string& year, const std::string& range) {
    size_t pos = range.find("->");
    if (pos != string::npos) {
        int startYear = stoi(range.substr(0, pos - 1));
        int endYear = stoi(range.substr(pos + 2));
        int yearNum = stoi(year);
        return yearNum >= startYear && yearNum <= endYear;
    }
    return false;
}

void processData(const std::string& filename, bool isMultipleYears, bool folder) {
    std::fstream file(filename, std::ios::in);
    if (!file.is_open()) {
        std::cout << "Error: Could not open file " << filename << std::endl;
        return;
    }

    history.push_back("Processing file: -----------------" + filename + "-----------------");

    std::vector<std::vector<std::string>> data;
    std::string line;

    // Read the first line to determine if it's a header
    std::getline(file, line);
    std::vector<std::string> header;
    std::string cell;
    size_t start = 0;
    size_t end = line.find(',');

    while (end != std::string::npos) {
        cell = line.substr(start, end - start);
        header.push_back(cell);
        start = end + 1;
        end = line.find(',', start);
    }
    cell = line.substr(start, end);
    header.push_back(cell);

    // Find the index of the year column
    int yearColIndex = 2;

    // Process the rest of the data
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        size_t start = 0;
        size_t end = line.find(',');

        while (end != std::string::npos) {
            cell = line.substr(start, end - start);
            row.push_back(cell);
            start = end + 1;
            end = line.find(',', start);
        }
        cell = line.substr(start, end);
        row.push_back(cell);

        // Skip rows with missing values (empty strings)
        bool hasMissingValue = false;
        for (const std::string& val : row) {
            if (val.empty()) {
                hasMissingValue = true;
                break;
            }
        }

        if (hasMissingValue) {
            continue;
        }

        if (isMultipleYears) {
            // Check if the row contains a valid year column
            if (row.size() > yearColIndex && !row[yearColIndex].empty()) {
                data.push_back(row);
            }
        }
        else {
            data.push_back(row);
        }
    }
    file.close();

    if (isMultipleYears) {
        // Display all available years, ensuring each year is unique
        std::vector<std::string> uniqueYears;
        for (const auto& row : data) {
            if (row.size() > yearColIndex) {
                std::string year = row[yearColIndex];
                // Add year if it is not already in the uniqueYears vector
                if (find(uniqueYears.begin(), uniqueYears.end(), year) == uniqueYears.end()) {
                    uniqueYears.push_back(year);
                }
            }
        }

        std::string chosenYear;

        if (folder) {
            chosenYear = "all";
        }
        else {
            cout << "Available years: ";
            for (const auto& year : uniqueYears) {
                cout << year << " ";
            }
            cout << endl;


            cout << "Enter 'all' to analyze all data, or specify a year or a range (e.g., 2020 -> 2023): ";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            getline(cin, chosenYear);

        }        

        // Check if the chosen year or range is valid
        if (chosenYear != "all" && chosenYear.find("->") == string::npos &&
            find(uniqueYears.begin(), uniqueYears.end(), chosenYear) == uniqueYears.end()) {
            cout << "Error: Year not found in the file. Please try again." << endl;
            return;
        }

        // Filter data by the chosen year or range
        std::vector<std::vector<std::string>> filteredData;
        for (const auto& row : data) {
            if (row.size() > static_cast<size_t>(yearColIndex)) {
                std::string year = row[yearColIndex];
                if (chosenYear == "all" || (chosenYear.find("->") != string::npos &&
                    isInYearRange(year, chosenYear)) || year == chosenYear) {
                    filteredData.push_back(row);
                }
            }
        }
        data = filteredData;
        history.push_back("Analyzing year or range: " + chosenYear);
    }

    int solarColIndex = 5;
    std::vector<std::string> results;

    convertMJToKWh(data, solarColIndex);
    writeConvertedData(filename, data);
    findMinMaxMean(data, solarColIndex, results);
    findMonthlyStats(data, solarColIndex, results);

    results.insert(results.begin(), filename);
    writeResultsToCSV("Results/results.csv", results);
}


// Converts MJ/m^2 to kWh/m^2 for all solar exposure values
void convertMJToKWh(std::vector<std::vector<std::string>>& data, int solarColIndex) {
    for (size_t i = 1; i < data.size(); ++i) {
        try {
            double mjPerM2 = std::stod(data[i][solarColIndex]);
            double kwhPerM2 = mjPerM2 / 3.6;
            data[i][solarColIndex] = std::to_string(kwhPerM2);
        }
        catch (const std::invalid_argument&) {
            std::cerr << "Invalid data at row " << i << ", column " << solarColIndex << std::endl;
        }
    }
}

// Writes the converted data to a new file with _kWhm^-2 suffix
void writeConvertedData(const string& filename, const vector<vector<string>>& data) {
    string newFilename = filename.substr(0, filename.find_last_of('.')) + "_kWhm^-2.csv";
    fstream outFile(newFilename);

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i];
            if (i < row.size() - 1) {
                outFile << ",";
            }
        }
        outFile << "\n";
    }

    outFile.close();
    cout << "Converted data written to: " << newFilename << endl;
    logInteraction("Wrote converted data to " + newFilename);
}

// Logs the interaction with the terminal to log.txt
void logInteraction(const string& message) {
    fstream logFile("Results/log.txt", ios_base::app);
    logFile << message << endl;
    logFile.close();
}

// Bubble sort algorithm for sorting a vector
void bubbleSort(std::vector<double>& vec) {
    size_t n = vec.size();
    bool swapped;
    do {
        swapped = false;
        for (size_t i = 1; i < n; ++i) {
            if (vec[i - 1] > vec[i]) {
                std::swap(vec[i - 1], vec[i]);
                swapped = true;
            }
        }
        --n;
    } while (swapped);
}

void findMinMaxMean(const vector<vector<string>>& data, int solarColIndex, vector<string>& results) {
    int totalRows = data.size();
    int totalColumns = (totalRows > 0) ? data[0].size() : 0;

    cout << "Total number of rows: " << totalRows << "\n";
    cout << "Total number of columns: " << totalColumns << "\n";

    logInteraction("Total rows: " + to_string(totalRows));
    logInteraction("Total columns: " + to_string(totalColumns));

    double maxExposure = numeric_limits<double>::lowest();
    double minExposure = numeric_limits<double>::max();
    double sumExposure = 0;
    std::vector<double> exposures;

    for (size_t i = 1; i < totalRows; ++i) {
        double exposure = stod(data[i][solarColIndex]);
        sumExposure += exposure;
        exposures.push_back(exposure);

        if (exposure > maxExposure) {
            maxExposure = exposure;
        }
        if (exposure < minExposure) {
            minExposure = exposure;
        }
    }

    double meanExposure = sumExposure / (totalRows - 1);

    // Calculate median
    bubbleSort(exposures);
    double medianExposure;
    size_t mid = exposures.size() / 2;
    if (exposures.size() % 2 == 0) {
        medianExposure = (exposures[mid - 1] + exposures[mid]) / 2.0;
    }
    else {
        medianExposure = exposures[mid];
    }

    // Calculate standard deviation
    double sumSquaredDiff = 0;
    for (double exposure : exposures) {
        sumSquaredDiff += pow(exposure - meanExposure, 2);
    }
    double standardDeviation = sqrt(sumSquaredDiff / (totalRows - 1));

    // Find percentage of days within one standard deviation of the mean
    int countWithinOneSD = 0;
    for (double exposure : exposures) {
        if (exposure >= meanExposure - standardDeviation && exposure <= meanExposure + standardDeviation) {
            countWithinOneSD++;
        }
    }
    double percentageWithinOneSD = (static_cast<double>(countWithinOneSD) / (totalRows - 1)) * 100;

    cout << "Percentage of days within one standard deviation of the mean: " << percentageWithinOneSD << "%\n";

    // Log the results
    logInteraction("Max exposure: " + to_string(maxExposure));
    logInteraction("Min exposure: " + to_string(minExposure));
    logInteraction("Mean exposure: " + to_string(meanExposure));
    logInteraction("Median exposure: " + to_string(medianExposure));
    logInteraction("Standard deviation: " + to_string(standardDeviation));
    logInteraction("Percentage of days within one SD: " + to_string(percentageWithinOneSD) + "%");

    // Save results
    results.push_back(to_string(totalRows));
    results.push_back(to_string(totalColumns));
    results.push_back(to_string(meanExposure));
    results.push_back(to_string(medianExposure));
    results.push_back(to_string(standardDeviation));
    results.push_back(to_string(percentageWithinOneSD) + "%");

    history.push_back("Max exposure: " + to_string(maxExposure));
    history.push_back("Min exposure: " + to_string(minExposure));
    history.push_back("Mean exposure: " + to_string(meanExposure));
    history.push_back("Median exposure: " + to_string(medianExposure));
    history.push_back("Standard deviation: " + to_string(standardDeviation));
    history.push_back("Percentage of days within one SD: " + to_string(percentageWithinOneSD) + "%");
}


// Function to find mode and its frequency
void findMode(const std::vector<double>& exposures) {
    // Determine the range of values for better precision
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    for (double value : exposures) {
        if (value < minVal) minVal = value;
        if (value > maxVal) maxVal = value;
    }

    // Define a vector to count occurrences of values
    int range = static_cast<int>(std::round(maxVal - minVal)) + 1;
    std::vector<int> frequency(range, 0);

    // Populate the frequency vector
    for (double value : exposures) {
        int index = static_cast<int>(std::round(value - minVal));
        if (index >= 0 && index < range) {
            frequency[index]++;
        }
    }

    // Find the mode
    double mode = std::numeric_limits<double>::quiet_NaN();
    int maxCount = 0;

    for (int i = 0; i < range; ++i) {
        if (frequency[i] > maxCount) {
            maxCount = frequency[i];
            mode = minVal + i;
        }
    }

    std::cout << "Mode of solar exposure: " << mode << " kWh/m^2\n";
    std::cout << "Frequency of mode: " << maxCount << " times\n";

    logInteraction("Mode exposure: " + std::to_string(mode));
    logInteraction("Frequency of mode: " + std::to_string(maxCount));

    history.push_back("Mode exposure: " + std::to_string(mode));
    history.push_back("Frequency of mode: " + std::to_string(maxCount));

}


// Converts year, month, day to dd/mm/yyyy format
string getDate(int year, int month, int day) {
    return to_string(day) + "/" + to_string(month) + "/" + to_string(year);
}

struct MonthlyData {
    string monthYear;
    vector<double> exposures;
};

void findMonthlyStats(const vector<vector<string>>& data, int solarColIndex, vector<string>& results) {
    vector<MonthlyData> monthlyData;

    // Collect monthly data
    for (size_t i = 1; i < data.size(); ++i) {
        string monthYear = data[i][3] + "/" + data[i][2];
        double exposure = stod(data[i][solarColIndex]);

        // Check if monthYear already exists
        auto it = find_if(monthlyData.begin(), monthlyData.end(), [&](const MonthlyData& md) {
            return md.monthYear == monthYear;
            });

        if (it != monthlyData.end()) {
            it->exposures.push_back(exposure);
        }
        else {
            MonthlyData newData;
            newData.monthYear = monthYear;
            newData.exposures.push_back(exposure);
            monthlyData.push_back(newData);
        }
    }

    double minMean = numeric_limits<double>::max();
    double maxMean = numeric_limits<double>::lowest();
    double minMedian = numeric_limits<double>::max();
    double maxMedian = numeric_limits<double>::lowest();
    double minVariance = numeric_limits<double>::max();

    string minMeanDate, maxMeanDate, minMedianDate, maxMedianDate, mostConsistentDate;

    for (const auto& md : monthlyData) {
        const vector<double>& exposures = md.exposures;
        double sum = 0;
        double sumSquares = 0;

        for (double val : exposures) {
            sum += val;
            sumSquares += val * val;
        }

        double mean = sum / exposures.size();

        // Compute median
        vector<double> sortedExposures = exposures;
        bubbleSort(sortedExposures);
        double median;
        size_t mid = sortedExposures.size() / 2;
        if (sortedExposures.size() % 2 == 0) {
            median = (sortedExposures[mid - 1] + sortedExposures[mid]) / 2.0;
        }
        else {
            median = sortedExposures[mid];
        }

        // Compute variance
        double variance = (sumSquares / exposures.size()) - (mean * mean);

        if (mean < minMean) {
            minMean = mean;
            minMeanDate = md.monthYear;
        }
        if (mean > maxMean) {
            maxMean = mean;
            maxMeanDate = md.monthYear;
        }
        if (median < minMedian) {
            minMedian = median;
            minMedianDate = md.monthYear;
        }
        if (median > maxMedian) {
            maxMedian = median;
            maxMedianDate = md.monthYear;
        }
        if (variance < minVariance) {
            minVariance = variance;
            mostConsistentDate = md.monthYear;
        }
    }

    cout << fixed;
    cout << "Month/Year with lowest mean exposure: " << minMeanDate << " with mean " << minMean << " kWh/m^2\n";
    cout << "Month/Year with highest mean exposure: " << maxMeanDate << " with mean " << maxMean << " kWh/m^2\n";
    cout << "Month/Year with lowest median exposure: " << minMedianDate << " with median " << minMedian << " kWh/m^2\n";
    cout << "Month/Year with highest median exposure: " << maxMedianDate << " with median " << maxMedian << " kWh/m^2\n";
    cout << "Month/Year with most consistent exposure: " << mostConsistentDate << " with variance " << minVariance << "\n";

    logInteraction("Month/Year with lowest mean exposure: " + minMeanDate + " with mean " + to_string(minMean));
    logInteraction("Month/Year with highest mean exposure: " + maxMeanDate + " with mean " + to_string(maxMean));
    logInteraction("Month/Year with lowest median exposure: " + minMedianDate + " with median " + to_string(minMedian));
    logInteraction("Month/Year with highest median exposure: " + maxMedianDate + " with median " + to_string(maxMedian));
    logInteraction("Month/Year with most consistent exposure: " + mostConsistentDate + " with variance " + to_string(minVariance));

    results.push_back(minMeanDate);
    results.push_back(maxMeanDate);
    results.push_back(minMedianDate);
    results.push_back(maxMedianDate);
    results.push_back(mostConsistentDate);

    history.push_back("Month/Year with lowest mean exposure: " + minMeanDate + " with mean " + to_string(minMean));
    history.push_back("Month/Year with highest mean exposure: " + maxMeanDate + " with mean " + to_string(maxMean));
    history.push_back("Month/Year with lowest median exposure: " + minMedianDate + " with median " + to_string(minMedian));
    history.push_back("Month/Year with highest median exposure: " + maxMedianDate + " with median " + to_string(maxMedian));
    history.push_back("Month/Year with most consistent exposure: " + mostConsistentDate + " with variance " + to_string(minVariance));


}

void writeResultsToCSV(const string& filename, const vector<string>& results) {
    // Check if the file is empty
    ifstream checkFile(filename, ios_base::in | ios_base::binary);
    bool isEmpty = checkFile.peek() == ifstream::traits_type::eof();
    checkFile.close();

    // Open the file in append mode
    ofstream resultFile(filename, ios_base::app);

    // Write headers if the file is empty
    if (isEmpty) {
        resultFile << "File Name,Number of Rows,Number of Columns,Date of Max Exposure,Date of Min Exposure,Mean Exposure,Median Exposure,Month/Year with lowest mean exposure,Month/Year with highest mean exposure,Month/Year with lowest median exposure,Month/Year with highest median exposure,Month/Year with most consistent exposure\n";
    }

    resultFile << results[0] << ",";
    for (size_t i = 1; i < results.size(); ++i) {
        resultFile << results[i];
        if (i < results.size() - 1) {
            resultFile << ",";
        }
    }
    resultFile << "\n";

    resultFile.close();
    cout << "Results written to: " << filename << endl;
    logInteraction("Wrote results to " + filename);
}

void displayPreviousCalculations() {
    if (history.empty()) {
        cout << "No previous calculations available." << endl;
        return;
    }

    cout << "Previous calculations:\n";
    for (const auto& result : history) {
        cout << "\t" + result << endl;
    }
}