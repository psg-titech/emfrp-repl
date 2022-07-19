# Is RxPy Runnable on MicroPython?
*The Answer is NO.*
RxPy cannot run on MicroPython.(2022/7/14)

The output is here:  
RxPy 4.0.x  

```python
>>> from reactivex import create
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "reactivex/__init__.py", line 380
SyntaxError: invalid syntax
```

RxPy 3.2.x  

```python
>>> import rx
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "rx/__init__.py", line 3, in <module>
ImportError: no module named 'asyncio'
```

RxPy 1.1.x  

```python
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
  File "rx/__init__.py", line 4, in <module>
  File "rx/internal/__init__.py", line 2, in <module>
  File "rx/internal/basic.py", line 1, in <module>
ImportError: no module named 'datetime'
```

lack of libraries. hmm...
