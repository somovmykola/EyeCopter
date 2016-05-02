#!/usr/bin/env python
import csv
import sys
import os
import Tkinter
import subprocess
from Tkinter import *

top = Tkinter.Tk()

L1 = Label(top, text="Picture Location")
L2 = Label(top, text="Picture List File Name")
L3 = Label(top, text="Model Output Location")
L4 = Label(top, text="Model Output File Name")
E1 = Entry(top, bd =1)
E2 = Entry(top, bd=1)
E3 = Entry(top, bd=1)
E4 = Entry(top, bd=1)

L1.grid(row=0,column=0)
E1.grid(row=1, column=0)
L2.grid(row=2, column=0)
E2.grid(row=3, column=0)
L3.grid(row=4, column=0)
E3.grid(row=5, column=0)
L4.grid(row=6, column=0)
E4.grid(row=7, column=0)

top.title("OSTRICH")
top.geometry("350x200")
def getDest():
  pic_location = E1.get()
  pic_listname = E2.get()
  mod_location = E3.get()
  mod_filename = E4.get()
  os.system("cmd /c VisualSFM sfm+cmvs " + pic_location + "/" + pic_listname + ".txt" + " " + mod_location + "/" + mod_filename + ".nvm")
  os.system("cmd /c VisualSFM sfm+loadnvm+pmvs " + mod_location + "/" + mod_filename + ".nvm" + " results/dense.nvm")


button = Tkinter.Button(top, text = "Model", command = getDest)
button.grid(row=3, column=0)

L1.pack()
E1.pack()
L2.pack()
E2.pack()
L3.pack()
E3.pack()
L4.pack()
E4.pack()
button.pack()

top.mainloop()
