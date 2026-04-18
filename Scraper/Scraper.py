from selenium import webdriver
from selenium.webdriver.chrome.service import Service
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from bs4 import BeautifulSoup
import time

driver_path = 'C:/webdriver/chromedriver.exe'
session = Service(driver_path)
option = webdriver.ChromeOptions()
#option.add_argument('--headless') #make the browser invisible
driver = webdriver.Chrome(service=session, options=option)
wait = WebDriverWait(driver, 10)


def get_number_of_pages():
    driver.get("https://www.maxongroup.com/maxon/view/category/prodfilter/gear?pn_id=ProductSearch&pn_p=1")
    wait.until(EC.presence_of_element_located((By.CLASS_NAME, 'paginator')))
    page_source = driver.page_source
    soup = BeautifulSoup(page_source,'lxml')
    number_of_pages = int(soup.find('div', attrs={'class': 'paginator'}).find_all('a')[-2].text)
    return number_of_pages
#get the number of pages to loop through

def get_articles_urls():
    articles_urls = []
    with open('product_urls.txt', 'w', encoding='utf-8') as file:
        for page_number in range(1, get_number_of_pages() + 1):
            driver.get(f"https://www.maxongroup.com/maxon/view/category/prodfilter/gear?pn_id=ProductSearch&pn_p={page_number}")
            wait.until(EC.presence_of_element_located((By.XPATH, '//table/tbody/tr')))
            page_source = driver.page_source
            page = BeautifulSoup(page_source,'lxml')
            table = page.find('table').find('tbody').find_all('tr')
            for row in table:
                article = row['data-detailurl']
                print(article)
                file.write("https://www.maxongroup.com" + article + '\n')
                articles_urls.append(article)
    return articles_urls
#get the urls of the articles and save them in a text file


def data_to_csv(gearHeadType,outerDiameter,version,reductionRatio,numberOfStages,maxContinuousTorque,maxIntermittentTorque,directionOfRotation,maxEfficiency,gearHeadLength,bearingAtOutputShaft,maxRadialLoad,maxAxialLoad,maxContinuousInputSpeed,maxIntermittentInputSpeed,recommendedTempRange,weight):
    with open('gears_details.csv', 'a', encoding='utf-8') as file:
        file.write(f"{gearHeadType},{outerDiameter},{version},{reductionRatio},{numberOfStages},{maxContinuousTorque},{maxIntermittentTorque},{directionOfRotation},{maxEfficiency},{gearHeadLength},{bearingAtOutputShaft},{maxRadialLoad},{maxAxialLoad},{maxContinuousInputSpeed},{maxIntermittentInputSpeed},{recommendedTempRange},{weight}\n")
#get the details of the articles and save them in a csv file

def get_articles_details():
    with open('gears_urls.txt', 'r', encoding='utf-8') as file:
        for line in file:
            driver.get(line.strip())
            wait.until(EC.presence_of_element_located((By.CLASS_NAME, 'iTabSpecifications')))
            time.sleep(1)
            page_source = driver.page_source
            soup = BeautifulSoup(page_source,'lxml')
            tables = soup.find('div', attrs={'class': 'iTabSpecifications'}).find('table').find_all('tbody')

            items = tables[1].find_all('td')
            if len(items) == 6:
                gearHeadType = items[1].text
                outerDiameter = items[3].text
                version = items[5].text
            elif len(items) == 4:
                gearHeadType = items[1].text
                outerDiameter = "N/A"
                version = items[3].text
            
            items = tables[3].find_all('td')
            if len(items) == 16:
                reductionRatio = items[1].text
                numberOfStages = items[5].text
                maxContinuousTorque = items[7].text
                maxIntermittentTorque = items[9].text
                directionOfRotation = items[11].text
                maxEfficiency = items[13].text
                gearHeadLength = items[15].text
            elif len(items) == 14 and '%' in items[11].text:
                reductionRatio = items[1].text
                numberOfStages = items[3].text
                maxContinuousTorque = items[5].text
                maxIntermittentTorque = items[7].text
                directionOfRotation = items[9].text
                maxEfficiency = items[11].text
                gearHeadLength = items[13].text
            elif len(items) == 14 and '%' not in items[11].text:
                reductionRatio = items[1].text
                numberOfStages = items[5].text
                maxContinuousTorque = items[7].text
                maxIntermittentTorque = items[9].text
                directionOfRotation = items[11].text
                maxEfficiency = "N/A"
                gearHeadLength = items[13].text
            elif len(items) == 12:
                reductionRatio = items[1].text
                numberOfStages = items[3].text
                maxContinuousTorque = items[5].text
                maxIntermittentTorque = items[7].text
                directionOfRotation = items[9].text
                maxEfficiency = "N/A"
                gearHeadLength = items[11].text
            
            items = tables[5].find_all('td')
            if len(items) == 12:
                bearingAtOutputShaft = items[1].text
                maxRadialLoad = items[3].text
                maxAxialLoad = items[5].text
                maxContinuousInputSpeed = items[7].text
                maxIntermittentInputSpeed = items[9].text
                recommendedTempRange = items[11].text
            elif len(items) == 10:
                bearingAtOutputShaft = items[1].text
                maxRadialLoad = items[3].text
                maxAxialLoad = items[5].text
                maxContinuousInputSpeed = "N/A"
                maxIntermittentInputSpeed = items[7].text
                recommendedTempRange = items[9].text
            items = tables[7].find_all('td')
            weight = items[1].text
            
            print(gearHeadType,outerDiameter,version,reductionRatio,numberOfStages,maxContinuousTorque,maxIntermittentTorque,directionOfRotation,maxEfficiency,gearHeadLength,bearingAtOutputShaft,maxRadialLoad,maxAxialLoad,maxContinuousInputSpeed,maxIntermittentInputSpeed,recommendedTempRange,weight)
            data_to_csv(gearHeadType,outerDiameter,version,reductionRatio,numberOfStages,maxContinuousTorque,maxIntermittentTorque,directionOfRotation,maxEfficiency,gearHeadLength,bearingAtOutputShaft,maxRadialLoad,maxAxialLoad,maxContinuousInputSpeed,maxIntermittentInputSpeed,recommendedTempRange,weight)
get_articles_details()

