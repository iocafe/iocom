
class MyTableData():
    def __init__(self, **kwargs):
        self.title = None

    def set_title(self, title):
        self.title = title
   

#class MyTableData:
#    calculate layout(resolution, options)
#    get number of: column header rows, data rows, summary rows 
#    get number of: row header columns, data columns 
#    get row height (panel) for: column header, data, summary and insert row
#    get column width for: each column

#    get header, summary or data value as string (panel, x, y)
#    set row header or data value as string (panel, x, y)

#    insert row
#    delete row
#    update row



def test_table_data():
    td = MyTableData()

if __name__ == '__main__':
    MainApp().run()