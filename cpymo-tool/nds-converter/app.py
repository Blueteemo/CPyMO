"""CPyMO NDS conversion assistant UI, MIT. NDS port: 千岚之夏."""
from pathlib import Path
import argparse, contextlib, datetime, json, os, subprocess, sys, tempfile
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from helper_core import run_job

from release_info import VERSION

class App:
    def __init__(self,window,state_root=None,self_test=None):
        self.window=window;self.process=None;self.self_test=self_test;self.test_started=False
        self.state_root=Path(state_root or Path(os.environ.get('LOCALAPPDATA',tempfile.gettempdir()))/'CPyMO-NDS-转换助手'/'jobs')
        self.source=tk.StringVar();self.output=tk.StringVar(value=str(Path.home()/'Desktop'))
        self.status=tk.StringVar(value='先选择游戏压缩包或游戏文件夹。')
        self.candidates=[];self.serial=0;self.last_result=None;self.last_output=None
        window.title('CPyMO NDS 转换助手');window.geometry('840x780');window.minsize(760,740)
        window.option_add('*Font',('Microsoft YaHei UI',10))
        style=ttk.Style();style.theme_use('clam');style.configure('TButton',padding=(12,7));style.configure('Title.TLabel',font=('Microsoft YaHei UI',19,'bold'))
        frame=ttk.Frame(window,padding=22);frame.pack(fill='both',expand=True)
        ttk.Label(frame,text='CPyMO NDS 转换助手',style='Title.TLabel').pack(anchor='w')
        ttk.Label(frame,text='选择游戏包，生成可复制到 R4 的游戏目录。无需另装 Python 或 FFmpeg。').pack(anchor='w',pady=(6,16))
        ttk.Label(frame,text='1  选择游戏',font=('Microsoft YaHei UI',11,'bold')).pack(anchor='w')
        row=ttk.Frame(frame);row.pack(fill='x',pady=(7,8))
        ttk.Entry(row,textvariable=self.source,state='readonly').pack(side='left',fill='x',expand=True)
        self.zip_button=ttk.Button(row,text='选压缩包',command=self.pick_zip);self.zip_button.pack(side='left',padx=(8,4))
        self.folder_button=ttk.Button(row,text='选文件夹',command=self.pick_folder);self.folder_button.pack(side='left')
        self.games=tk.Listbox(frame,height=3,exportselection=False,activestyle='none',selectbackground='#27659c',relief='solid',borderwidth=1)
        self.games.pack(fill='x');ttk.Label(frame,text='支持 S60v3 / S60v5 / 安卓 PyMO 数据；同包有多个版本时优先推荐 S60v3。',foreground='#526579').pack(anchor='w',pady=(5,12))
        ttk.Label(frame,text='2  选择保存位置',font=('Microsoft YaHei UI',11,'bold')).pack(anchor='w')
        row=ttk.Frame(frame);row.pack(fill='x',pady=(7,6));ttk.Entry(row,textvariable=self.output,state='readonly').pack(side='left',fill='x',expand=True)
        self.output_button=ttk.Button(row,text='选保存位置',command=self.pick_output);self.output_button.pack(side='left',padx=(8,0))
        ttk.Label(frame,text='自动创建新文件夹，不覆盖原包或已有存档。视频默认省略并安全跳过。',foreground='#526579').pack(anchor='w',pady=(0,10))
        self.box=tk.BooleanVar(value=True)
        ttk.Checkbutton(frame,text='遇到字库缺字时显示 □ 并记录，保留原脚本内容',variable=self.box).pack(anchor='w')
        row=ttk.Frame(frame);row.pack(fill='x',pady=(13,8))
        self.convert_button=ttk.Button(row,text='3  开始转换',command=self.begin,state='disabled');self.convert_button.pack(side='left')
        self.cancel_button=ttk.Button(row,text='取消',command=self.cancel,state='disabled');self.cancel_button.pack(side='left',padx=8)
        self.open_button=ttk.Button(row,text='打开结果文件夹',command=self.open_output,state='disabled');self.open_button.pack(side='right')
        self.progress=ttk.Progressbar(frame,mode='indeterminate');self.progress.pack(fill='x')
        ttk.Label(frame,textvariable=self.status,wraplength=790).pack(anchor='w',pady=(8,4))
        self.log=tk.Text(frame,height=4,wrap='word',background='#f1f4f8',relief='flat',font=('Microsoft YaHei UI',9));self.log.pack(fill='both',expand=True,pady=(4,8))
        ttk.Label(frame,text='NDS 平台移植：千岚之夏  ·  基于 CPyMO / PyMO  ·  转换助手 '+VERSION,foreground='#526579').pack(anchor='w')
        window.protocol('WM_DELETE_WINDOW',self.close)
        if self_test:
            self.source.set(self_test['source']);self.output.set(self_test['output_base']);window.after(300,self.inspect)

    def pick_zip(self):
        p=filedialog.askopenfilename(title='选择 PyMO 游戏包',filetypes=[('游戏包','*.zip *.apk'),('全部文件','*.*')])
        if p:self.source.set(p);self.inspect()

    def pick_folder(self):
        p=filedialog.askdirectory(title='选择含 gameconfig.txt 的游戏数据目录')
        if p:self.source.set(p);self.inspect()

    def pick_output(self):
        p=filedialog.askdirectory(title='选择保存位置')
        if p:self.output.set(p)

    def busy(self,value):
        for button in (self.zip_button,self.folder_button,self.output_button):button.configure(state='disabled' if value else 'normal')
        self.cancel_button.configure(state='normal' if value else 'disabled')
        self.convert_button.configure(state='normal' if not value and self.candidates else 'disabled')
        if value:self.progress.start(12)
        else:self.progress.stop()

    def start(self,job,callback):
        self.serial+=1;stamp=datetime.datetime.now().strftime('%Y%m%d-%H%M%S-%f');folder=self.state_root/(stamp+'-'+str(self.serial));folder.mkdir(parents=True)
        self.job_file=folder/'job.json';self.result_file=folder/'result.json';self.log_file=folder/'log.txt'
        job.update(result=str(self.result_file.resolve()),log=str(self.log_file.resolve()));self.job_file.write_text(json.dumps(job,ensure_ascii=False,indent=2),encoding='utf8')
        cmd=[sys.executable]+([] if getattr(sys,'frozen',False) else [str(Path(__file__).resolve())])+['--job',str(self.job_file.resolve())]
        self.process=subprocess.Popen(cmd,stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=0x08000000 if os.name=='nt' else 0)
        self.callback=callback;self.busy(True);self.window.after(200,self.poll)

    def poll(self):
        if not self.process:return
        if self.log_file.exists():
            text=self.log_file.read_text(encoding='utf8',errors='replace');self.log.delete('1.0','end');self.log.insert('end',text[-5000:]);self.log.see('end')
        if self.process.poll() is None:self.window.after(300,self.poll);return
        self.process=None;self.busy(False)
        report=json.loads(self.result_file.read_text(encoding='utf8')) if self.result_file.exists() else {'ok':False,'error':'转换进程意外退出，请保留诊断日志。'}
        self.last_result=report;self.callback(report)

    def inspect(self):
        self.candidates=[];self.games.delete(0,'end');self.status.set('正在识别游戏包里的平台版本……')
        self.start({'action':'inspect','source':self.source.get()},self.inspected)

    def inspected(self,report):
        if not report['ok']:self.failed(report);return
        self.candidates=report['candidates']
        for i,c in enumerate(self.candidates):self.games.insert('end',f"{'推荐  ' if i==0 else ''}[{c['platform']}]  {c['title']}")
        self.games.selection_set(0);self.convert_button.configure(state='normal');self.status.set('已识别游戏。确认保存位置后，点击“开始转换”。')
        if self.self_test and not self.test_started:self.test_started=True;self.window.after(200,self.begin)

    def begin(self):
        selected=self.games.curselection()
        if not selected:return
        source=Path(self.source.get()).resolve();base=Path(self.output.get()).resolve()
        if source.is_dir() and base.is_relative_to(source):self.failed({'ok':False,'error':'保存位置不能位于原游戏目录内部。'});return
        name='CPyMO_NDS_转换结果_'+datetime.datetime.now().strftime('%Y%m%d_%H%M%S_%f');output=base/name
        self.status.set('正在转换，请稍候。语音较多的游戏需要更长时间……');self.open_button.configure(state='disabled')
        self.start({'action':'convert','source':str(source),'candidate':int(selected[0]),'output':str(output),'missing_policy':'box' if self.box.get() else 'error'},self.converted)

    def converted(self,report):
        if not report['ok']:self.failed(report);return
        self.last_output=Path(report['output']);self.open_button.configure(state='normal');self.status.set(report['status']+'。点击“打开结果文件夹”查看安装必读。')
        self.log.insert('end','\n'+('\n'.join(report['warnings']) if report['warnings'] else '未发现静态兼容性警告。')+'\n');self.log.see('end')
        if self.self_test:self.finish_test(report)

    def failed(self,report):
        self.status.set('未完成：'+report['error'])
        if self.self_test:self.finish_test(report)
        else:messagebox.showerror('暂时不能完成转换',report['error']+'\n\n原游戏包未改动。诊断目录：'+str(self.result_file.parent))

    def finish_test(self,report):
        def capture():
            self.window.lift();self.window.update()
            from PIL import ImageGrab
            box=(self.window.winfo_rootx(),self.window.winfo_rooty(),self.window.winfo_rootx()+self.window.winfo_width(),self.window.winfo_rooty()+self.window.winfo_height())
            ImageGrab.grab(bbox=box).save(self.self_test['screenshot'])
            Path(self.self_test['result']).write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf8');self.window.after(250,self.close)
        self.window.after(250,capture)

    def cancel(self):
        if self.process and self.process.poll() is None:
            if os.name=='nt':subprocess.run(['taskkill','/PID',str(self.process.pid),'/T','/F'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,creationflags=0x08000000)
            else:self.process.terminate()
        self.process=None;self.busy(False);self.status.set('已取消。原游戏包未改动；未完成目录和日志保留供检查。')

    def open_output(self):
        if self.last_output:os.startfile(self.last_output)

    def close(self):
        if self.process:self.cancel()
        self.window.destroy()

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--job',type=Path);parser.add_argument('--state-root',type=Path);parser.add_argument('--self-test',type=Path);args=parser.parse_args()
    if args.job:
        job=json.loads(args.job.read_text(encoding='utf8'))
        with Path(job['log']).open('w',encoding='utf8',buffering=1) as log,contextlib.redirect_stdout(log),contextlib.redirect_stderr(log):report=run_job(job)
        return 0 if report['ok'] else 1
    test=json.loads(args.self_test.read_text(encoding='utf8')) if args.self_test else None

    if os.name=='nt':
        import ctypes
        ctypes.windll.user32.SetProcessDPIAware()
    window=tk.Tk();App(window,args.state_root,test);window.mainloop();return 0

if __name__=='__main__':sys.exit(main())
