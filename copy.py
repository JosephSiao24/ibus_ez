# 指定源文件和目標文件的路徑
source = '/home/joseph/Desktop/HelloWorldProject/en_dictpp.csv'  # 請將此處替換為你的源文件路徑
destination = '/home/joseph/Desktop/HelloWorldProject/en_dictpk.csv'  # 請將此處替換為你的目標文件路徑

try:
    # 讀取源文件內容
    with open(source, 'r', encoding='utf-8') as src_file:
        content = src_file.read()
    
    # 將內容寫入目標文件
    with open(destination, 'w', encoding='utf-8') as dest_file:
        dest_file.write(content)
    
    print(f"成功將內容從 {source} 寫入到 {destination}")

except Exception as e:
    print(f"操作失敗: {e}")

