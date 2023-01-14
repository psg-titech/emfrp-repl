
# Title: Emfrp repl GUI frontend(Python/tk)
# Author: Go Suzuki <puyogo.suzuki@gmail.com>
# Date: 2023/1/14

import tkinter
import ctypes
import os
import platform

librarypath = None

if os.name == 'nt' or os.name == 'ce':
    librarypath = os.path.join(os.path.dirname(__file__), '..', 'project', 'pc', 'build', 'Debug', 'libemfrp-repl.dll')
    if not os.path.exists(librarypath):
        librarypath = os.path.join(os.path.dirname(__file__), '..', 'project', 'pc', 'build', 'Release', 'libemfrp-repl.dll')
elif os.name == 'posix':
    librarypath = os.path.join(os.path.dirname(__file__), '..', 'project', 'pc', 'build', 'libemfrp-repl.so')
    if platform.system() == 'Darwin':
        librarypath = os.path.join(os.path.dirname(__file__), '..', 'project', 'pc', 'build', 'libemfrp-repl.dylib')
        print('macOS\'s tkinter is very buggy, we recommend other OSes.')
    elif platform.system() in ['Linux', 'DragonFly', 'FreeBSD', 'Haiku', 'NetBSD', 'OpenBSD', 'Solaris']:
        pass
    else:
        print('Not tested platform!')

libemfrp = ctypes.cdll.LoadLibrary(librarypath)
libemfrp.emfrp_create.restype = ctypes.c_int32
libemfrp.emfrp_create.argtypes = [ctypes.c_void_p]
libemfrp.emfrp_repl.restype = ctypes.c_int32
libemfrp.emfrp_repl.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p]
libemfrp.emfrp_add_input_node.restype = ctypes.c_int32
libemfrp.emfrp_add_input_node.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p]
libemfrp.emfrp_update.restype = ctypes.c_int32
libemfrp.emfrp_update.argtypes = [ctypes.c_void_p]
libemfrp.emfrp_add_output_node.restype = ctypes.c_int32
libemfrp.emfrp_add_output_node.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_void_p]
libemfrp.emfrp_create_int_object.restype = ctypes.c_void_p
libemfrp.emfrp_create_int_object.argtypes = [ctypes.c_int32]
libemfrp.emfrp_get_integer.restype = ctypes.c_int32
libemfrp.emfrp_get_integer.argtypes = [ctypes.c_void_p]
libemfrp.emfrp_print_object.argstype = [ctypes.c_void_p]
emfrp_t = ctypes.c_void_p(0)
libemfrp.emfrp_create(ctypes.pointer(emfrp_t))
root = tkinter.Tk()
root.title('emfrp-repl')

logText = tkinter.Text(root,state=tkinter.DISABLED)

commandFrame = tkinter.Frame(root)
commandFrame.pack()
commandVar = tkinter.StringVar()
def execCommand():
    s = commandVar.get()
    o = ctypes.c_void_p(0)
    op = ctypes.pointer(o)
    returnCode = libemfrp.emfrp_repl(emfrp_t, ctypes.c_char_p(bytes(s, 'ascii')), op)
    logText['state'] = tkinter.NORMAL
    logText.insert('end', '\n' + s)
    logText.insert('end', '\nReturn Code:' + str(returnCode))
    libemfrp.emfrp_print_object(o)
#    logText.insert('end', '\nReturn Value:' + str(libemfrp.emfrp_get_integer(o)))
    logText['state'] = tkinter.DISABLED
    commandVar.set('')

cmdEntry = tkinter.Entry(
    commandFrame, width=128, 
    textvariable=commandVar)
cmdEntry.bind('<Return>', lambda a: execCommand())
cmdEntry.pack(side=tkinter.LEFT)
tkinter.Button(
    commandFrame, text='Execute',
    command=execCommand).pack(side=tkinter.LEFT)


sliderFrame = tkinter.Frame(root)
sliderFrame.pack()

inputNodeNum = 0

def addInputNode():
    global inputNodeNum
    cont = tkinter.Frame(sliderFrame)
    cont.pack(side=tkinter.LEFT)
    val = tkinter.IntVar()
    txtVal = tkinter.StringVar()
    inputNodeNumStr = 'in' + str(inputNodeNum)
    txtVal.set(inputNodeNumStr + ' : 0')
    def inputNodeGet():
        return libemfrp.emfrp_create_int_object(val.get())
    GET_CALLBACK = ctypes.CFUNCTYPE(ctypes.c_void_p)
    # dont collect by gc!
    val.get_callback = GET_CALLBACK(inputNodeGet)
    logText.insert('end', '\nReturn Code:' + str(libemfrp.emfrp_add_input_node(emfrp_t, ctypes.c_char_p(bytes(inputNodeNumStr, 'ascii')), val.get_callback)))
    la = tkinter.Label(cont,textvariable=txtVal)
    def inputNodeSliderChanged(a):
        txtVal.set(inputNodeNumStr + ' : ' + str(val.get()))
        libemfrp.emfrp_set_node_value(emfrp_t, ctypes.c_char_p(bytes(inputNodeNumStr, 'ascii')), libemfrp.emfrp_create_int_object(val.get()))
        libemfrp.emfrp_update(emfrp_t)
    tkinter.Scale(
        cont,
        variable=val,
        orient=tkinter.VERTICAL,
        length=200,
        from_=0,
        to=100000,
        command=inputNodeSliderChanged
        ).pack()
    la.pack()
    inputNodeNum = inputNodeNum + 1

sliderAddButton = tkinter.Button(
    sliderFrame,
    text='Add Input Node',
    command = addInputNode
    )
sliderAddButton.pack()

valueFrame = tkinter.Frame(root)
valueFrame.pack()

outputNodeNum = 0
def addOutputNode():
    global outputNodeNum
    cont = tkinter.Frame(valueFrame)
    cont.pack(side=tkinter.LEFT)
    val = tkinter.IntVar()
    txtVal = tkinter.StringVar()
    outputNodeNumStr = 'out' + str(outputNodeNum)
    txtVal.set(outputNodeNumStr + ' : 0')
    def indicated(obj_ptr):
        v = libemfrp.emfrp_get_integer(obj_ptr)
        txtVal.set(outputNodeNumStr + ' : ' + str(v))
    INDICATE_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
    # dont collect by gc!
    txtVal.indicate_callback = INDICATE_CALLBACK(indicated)
    logText.insert('end', '\nReturn Code:' + str(libemfrp.emfrp_add_output_node(emfrp_t, ctypes.c_char_p(bytes(outputNodeNumStr, 'ascii')),
                                              txtVal.indicate_callback)))
    la = tkinter.Label(cont, textvariable=txtVal)
    la.pack()
    outputNodeNum = outputNodeNum + 1

outputNodeAddButton = tkinter.Button(
    valueFrame,
    text='Add Output Node',
    command = addOutputNode
    )
outputNodeAddButton.pack()

logText.pack()

root.mainloop()
