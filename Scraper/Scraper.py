from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from bs4 import BeautifulSoup
import json
import pandas as pd

# Path to ChromeDriver executable
driver_path = 'C:/webdriver/chromedriver.exe'

# Create Selenium Chrome service and browser options
session = Service(driver_path)
option = webdriver.ChromeOptions()
# option.add_argument('--headless')  # Run browser in background (no UI)

# Initialize WebDriver and explicit wait helper
driver = webdriver.Chrome(service=session, options=option)
wait = WebDriverWait(driver, 10)


def GetPageNumbers():
    """
    Open the first products page and read paginator
    to determine how many pages exist.
    """
    driver.get("https://www.maxongroup.com/maxon/view/category/prodfilter/motor?pn_id=ProductSearch&pn_p=1")

    # Wait until pagination is visible
    wait.until(EC.presence_of_element_located((By.CLASS_NAME, 'paginator')))

    # Parse HTML and extract the second-to-last paginator link as page count
    page_source = driver.page_source
    soup = BeautifulSoup(page_source, 'lxml')
    numberOfPages = int(
        soup.find('div', attrs={'class': 'paginator'}).find_all('a')[-2].text
    )
    return numberOfPages


def GetProductsUrls():
    """
    Iterate over all pages, extract each product URL,
    save them to motorsUrls.txt, and return them as a list.
    """
    urls = []

    with open('motorsUrls.txt', 'w', encoding='utf-8') as file:
        # Loop through all pages
        for pageNumber in range(1, GetPageNumbers() + 1):
            driver.get(
                f"https://www.maxongroup.com/maxon/view/category/prodfilter/motor?pn_id=ProductSearch&pn_p={pageNumber}"
            )

            # Wait for table rows to load
            wait.until(EC.presence_of_element_located((By.XPATH, '//table/tbody/tr')))

            pageSource = driver.page_source
            page = BeautifulSoup(pageSource, 'lxml')
            table = page.find('table').find('tbody').find_all('tr')

            for row in table:
                urlSuffix = row['data-detailurl']
                # Save full URL to file
                file.write("https://www.maxongroup.com" + urlSuffix + '\n')

                # Keep full URLs in list
                urls.append("https://www.maxongroup.com" + urlSuffix)

    return urls


def GetData():
    """
    Read URLs from motorsUrls.txt, scrape specification tables from each page,
    and append each product record to motorsDetails.json.
    Retries each URL up to 10 times on failure.
    """
    # Load all URLs from text file
    with open('motorsUrls.txt', 'r', encoding='utf-8') as file:
        fileNumber = file.readlines()

    # Process each URL line-by-line
    for index, url in enumerate(fileNumber):
        counter = 0

        # Retry loop for transient failures (timeouts, element not found, etc.)
        while True:
            try:
                driver.get(url)

                # Wait until specifications tab content appears
                wait.until(EC.presence_of_element_located((By.CLASS_NAME, 'iTabSpecifications')))

                pageSource = driver.page_source
                soup = BeautifulSoup(pageSource, 'lxml')
                tables = soup.find('div', attrs={'class': 'iTabSpecifications'}).find('table').find_all('tbody')

                data = {}
                data['index'] = index
                data['url'] = url.strip()

                # Extract key/value fields from alternating tbody blocks
                for i in range(1, len(tables) + 1, 2):
                    items = tables[i].find_all('td')
                    for i in range(0, len(items), 2):
                        item = items[i]
                        data[item.text] = items[i + 1].text

                # Success: exit retry loop
                break

            except Exception as e:
                counter += 1
                print(f"Error processing line {index}: {e}")

                # Stop retrying after 10 failures
                if counter >= 10:
                    print(f"Skipping line {index} after 10 failed attempts.")
                    break

        # Append current record to output JSON file
        with open('motorsDetails.json', 'a', encoding='utf-8') as file:
            json.dump(data, file, indent=4, ensure_ascii=False)
            file.write(',\n')  # Separator between records


def FinalizeJsonFile():
    """
    After all records are appended, read the file and wrap the content in an array.
    This ensures the JSON is properly formatted for loading.
    """
    with open('motorsDetails.json', 'r+', encoding='utf-8') as file:
        content = file.read().rstrip(',\n')  # Remove trailing comma and newline
        file.seek(0)
        file.write('[' + content + ']')  # Wrap in array brackets


def convert_json_to_csv():
    """
    Load JSON data from motorsDetails.json and export to CSV.
    """
    with open('motorsDetails.json', 'r', encoding='utf-8') as file:
        data = json.load(file)

    df = pd.json_normalize(data)
    # df = df.reindex(columns=keys)  # Ensure all keys are included as columns
    df.columns = df.columns.str.strip()
    df.fillna('N/A', inplace=True)  # Replace NaN with empty strings
    df.to_csv('motorsDetails.csv', index=False, encoding='utf-8')