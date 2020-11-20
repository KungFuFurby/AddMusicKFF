using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Linq;

namespace AMKGUI
{
	public partial class Form1 : Form
	{
		SongList globalSongs = null;
		SongList localSongs = null;
		string romPath = "";
		string listFolderPath = "";
		int minimalErrorFormHeight;
		int normalErrorFormHeight;

		public Form1()
		{
			InitializeComponent();
			minimalErrorFormHeight = Height + 100;
			normalErrorFormHeight = Height;
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if (keyData == (Keys.Control | Keys.R))
			{
				runAMK();
			}
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void Form1_Load(object sender, EventArgs e)
		{
		}

		private void Form1_Shown(object sender, EventArgs e)
		{
			parseListFiles();
		}

		private void parseListFiles()
		{
			try
			{
				string file = "";
				string filePath = "";
				if (listFolderPath != "")
					file = System.IO.File.ReadAllText(filePath = listFolderPath + "Addmusic_list.txt");
				else if (System.IO.File.Exists(System.IO.Path.GetFullPath("Addmusic_list.txt")) == false)
				{
					if (openListOpenFileDialog.ShowDialog() != System.Windows.Forms.DialogResult.OK)
					{
						MessageBox.Show("Quitting.");
						Close();
						return;
					}
					file = System.IO.File.ReadAllText(filePath = openListOpenFileDialog.FileName);
				}
				else
					file = System.IO.File.ReadAllText(filePath = System.IO.Path.GetFullPath("Addmusic_list.txt"));

				filePath = filePath.Replace('\\', '/');

				if (filePath.LastIndexOf("/") >= 0)
					listFolderPath = filePath.Substring(0, filePath.LastIndexOf("/")) + "/";

				globalSongs = SongList.parseSongs(file, "Globals");
				localSongs = SongList.parseSongs(file, "Locals");
			}
			catch (Exception ex)
			{
				MessageBox.Show("Quitting due to an error.\r\nHere is the message:\r\n\r\n----------------------------------------\r\n\r\n" + ex.Message, "", MessageBoxButtons.OK, MessageBoxIcon.Error);
				Close();
				return;
			}

			refreshListBoxes();
		}

		private void button1_Click(object sender, EventArgs e)
		{
			if (listBox1.SelectedIndex == -1) return;
			openSongOpenFileDialog.InitialDirectory = (listFolderPath + "music/").Replace('/', '\\');
			if (openSongOpenFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
				globalSongs.fileNames[listBox1.SelectedIndex + 1] = toRelativePath(openSongOpenFileDialog.FileName);

			refreshListBoxes();
		}

		private void button2_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return;

			openSongOpenFileDialog.InitialDirectory = (listFolderPath + "music/").Replace('/', '\\');
			if (openSongOpenFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				string filePath = openSongOpenFileDialog.FileName.Replace('\\', '/');
				string[] directories = filePath.Split('/');
				if (directories.Contains("music") == false)
				{
					MessageBox.Show("This music file was not in the music folder (or in a folder in the music folder, etc.)");
					return;
				}
				localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count] = toRelativePath(openSongOpenFileDialog.FileName).Replace('\\', '/');
			}

			refreshListBoxes();
		}

		private void button3_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return; 

			localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count] = "";

			refreshListBoxes();
		}

		private void button4_Click(object sender, EventArgs e)
		{
			openSongOpenFileDialog.InitialDirectory = (listFolderPath + "music/").Replace('/', '\\');
			if (openSongOpenFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
			{
				if (listBox2.Items.Count != 0)
				{
					var index = listBox2.SelectedIndex;
					if (index == -1)
						index = listBox2.Items.Count - 1;
					localSongs.fileNames.Insert(index + globalSongs.fileNames.Count + 1, toRelativePath(openSongOpenFileDialog.FileName).Replace('\\', '/'));
				}
				else
				{
					localSongs.fileNames.Resize(localSongs.fileNames.Count, "");
					localSongs.fileNames.Add(toRelativePath(openSongOpenFileDialog.FileName).Replace('\\', '/'));
				}
			}

			refreshListBoxes();
			listBox2.SelectedIndex++;
		}

		private string toRelativePath(string path)
		{
			if (System.IO.Path.IsPathRooted(listFolderPath))
				return path.Substring((listFolderPath + "music/").Length);
			else
				return "music/" + path;
		}

		private void refreshListBoxes()
		{
			int backUpIndex1 = listBox1.SelectedIndex;
			int backUpIndex2 = listBox2.SelectedIndex;
			listBox1.Items.Clear();
			listBox2.Items.Clear();

			for (int i = 1; i < globalSongs.fileNames.Count; i++)
			{
				listBox1.Items.Add(i.ToString("X2") + ":\t" + globalSongs.fileNames[i]);
			}

			for (int i = globalSongs.fileNames.Count; i < localSongs.fileNames.Count; i++)
			{
				listBox2.Items.Add(i.ToString("X2") + ":\t" + localSongs.fileNames[i]);
			}

			if (backUpIndex1 < listBox1.Items.Count)
				listBox1.SelectedIndex = backUpIndex1;

			if (backUpIndex2 < listBox2.Items.Count)
				listBox2.SelectedIndex = backUpIndex2;
		}

		private void button5_Click(object sender, EventArgs e)
		{
			runAMK();
		}

		static string amkOutput = "";
		static string amkError = "";

		void runAMK()
		{
			if (porterModeCheckBox.Checked == true && listBox2.SelectedIndex == -1)
			{
				MessageBox.Show("No song selected to compile.");
				return;
			}

			try
			{
				string writeStr = globalSongs.ToString() + localSongs.ToString();
				System.IO.File.WriteAllText(System.IO.Path.Combine(listFolderPath, "Addmusic_list.txt"), writeStr);

				System.Diagnostics.Process amk = new System.Diagnostics.Process();

				amk.StartInfo.CreateNoWindow = true;
				amk.StartInfo.UseShellExecute = false;

				amk.StartInfo.FileName = listFolderPath + "AddmusicK.exe";
				amk.StartInfo.WorkingDirectory = listFolderPath;

				if (porterModeCheckBox.Checked == false)
				{
					if (romPath == "")
					{
						romOpenFileDialog.InitialDirectory = listFolderPath.Replace('/', '\\');
						if (romOpenFileDialog.ShowDialog() != System.Windows.Forms.DialogResult.OK) return;
						romPath = romOpenFileDialog.FileName;
					}
					amk.StartInfo.Arguments = "\"" + romPath + "\"";
				}
				else
				{
					amk.StartInfo.Arguments = "-norom \"" + localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count] + "\"";
				}

				if (verboseAMKCheckBox.Checked == true)
					amk.StartInfo.Arguments += " -v";

				amk.StartInfo.Arguments += " -noblock";

				label3.Text = "Please wait a moment...";
				this.Refresh();


				amkError = "";
				amkOutput = "";


				amk.StartInfo.RedirectStandardOutput = true;
				amk.StartInfo.RedirectStandardError = true;

				amk.OutputDataReceived += new System.Diagnostics.DataReceivedEventHandler(AmkOutputDataHandler);
				amk.ErrorDataReceived += new System.Diagnostics.DataReceivedEventHandler(AmkErrorDataHandler);
				amk.Start();
				amk.BeginOutputReadLine();
				amk.BeginErrorReadLine();
				amk.WaitForExit();



				label3.Text = "";

				if (porterModeCheckBox.Checked == false)
				{
					(new ResultForm(amk.ExitCode == 0 ? amkOutput : amkError, amk.ExitCode == 0)).ShowDialog();
				}
				else
				{
					if (amk.ExitCode == 0)
					{
						string searchStr = "Completed in ";
						try
						{
							int index = amkOutput.IndexOf(searchStr) + searchStr.Length;
							int index2 = amkOutput.IndexOf(' ', index);
							double timeTaken = 0;
							if (index != -1)
							{
								string timeStr = amkOutput.Substring(index, index2 - index);
								timeTaken = Convert.ToDouble(timeStr);
								textBox1.Text = "Song compiled successfully (" + timeTaken.ToString() + (timeTaken == 1 ? " second)" : " seconds)");
							}
						}
						catch
						{
							textBox1.Text = "Song compiled successfully";
						}

						playSPC(listBox2.SelectedIndex + globalSongs.fileNames.Count);
					}
					else
					{
						textBox1.Text = "There were errors while compiling:\r\n\r\n" + amkError;
					}
				}
			}
			catch (Exception ex)
			{
				MessageBox.Show("Could not start AddmusicK.  The error was: " + ex.ToString());
				return;
			}

		}

		private static void AmkOutputDataHandler(object sendingProcess, System.Diagnostics.DataReceivedEventArgs outLine)
		{

			amkOutput += outLine.Data + Environment.NewLine;
		}

		private static void AmkErrorDataHandler(object sendingProcess, System.Diagnostics.DataReceivedEventArgs errLine)
		{
			amkError += errLine.Data + Environment.NewLine;
		}
		
		private string Read(System.IO.StreamReader reader)
		{
			string ret = "";
			new System.Threading.Thread(() =>
			{
				while (true)
				{
					int current;
					while ((current = reader.Read()) >= 0)
						ret += current;
				}
			}).Start();

			return ret;
		}

		private void button6_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return;
			playSPC(listBox2.SelectedIndex + globalSongs.fileNames.Count);
		}

		private void playSPC(int localSongIndex)
		{
			if (checkBox1.Checked == false) return;
			string spcFilePath = localSongs.fileNames[localSongIndex].Replace('\\', '/'); ;

			if (spcFilePath.LastIndexOf('/') >= 0)
				spcFilePath = spcFilePath.Substring(spcFilePath.LastIndexOf('/') + 1);
			if (spcFilePath.LastIndexOf('.') >= 0)
				spcFilePath = spcFilePath.Substring(0, spcFilePath.LastIndexOf('.')) + ".spc";

			spcFilePath = listFolderPath + "SPCs/" + spcFilePath;

			if (System.IO.File.Exists(spcFilePath) == false)
			{
				MessageBox.Show("The SPC for this song has not yet been generated. Click on the Run button first, then try again.");
				return;
			}

			try
			{
				System.Diagnostics.Process.Start(spcFilePath);
			}
			catch
			{
				MessageBox.Show("Could not play the SPC file. Please make sure that you have a program set up that can play SPC files, and that SPC files are set to play with that program automatically when opened.");
			}
		}

		private void listBox2_SelectedIndexChanged(object sender, EventArgs e)
		{
		}

		private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			button1.Enabled = (listBox1.SelectedIndex != -1);
		}

		private void timer1_Tick(object sender, EventArgs e)
		{
			button1.Enabled = (listBox1.SelectedIndex != -1);
			button8.Enabled = button6.Enabled = button3.Enabled = button2.Enabled = (listBox2.SelectedIndex != -1);
			button12.Enabled = (listBox2.SelectedIndex != 0 && listBox2.SelectedIndex != -1);
			button11.Enabled = (listBox2.SelectedIndex != listBox2.Items.Count - 1 && listBox2.SelectedIndex != -1);

			button4.Enabled = (listBox2.Items.Count == 0 || listBox2.SelectedIndex != 0);
		}

		private void checkBox1_CheckedChanged(object sender, EventArgs e)
		{
			//SuspendLayout();
			var backup = listBox2.SelectedIndex;
			if (porterModeCheckBox.Checked == true)
			{
				Height = minimalErrorFormHeight;
				splitContainer1.Panel2Collapsed = false;
				splitContainer1.FixedPanel = FixedPanel.None;
				splitContainer1.SplitterDistance = splitContainer1.Height - 100;
				checkBox1.Visible = true;
			}
			else
			{
				splitContainer1.Panel2Collapsed = true;
				Height = normalErrorFormHeight;
				splitContainer1.FixedPanel = FixedPanel.Panel2;
				checkBox1.Visible = false;
			}
			listBox2.SelectedIndex = backup;
			//ResumeLayout();
		}

		private void Form1_Resize(object sender, EventArgs e)
		{

		}

		private void button7_Click(object sender, EventArgs e)
		{
			parseListFiles();
		}





		private void button12_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return;
			string higherValueToReplaceWith = String.Copy(localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count - 1]);
			string lowerValueToReplaceWith = String.Copy(localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count]);
			localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count - 1] = lowerValueToReplaceWith;
			localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count] = higherValueToReplaceWith;

			listBox2.BeginUpdate();
			refreshListBoxes();
			listBox2.SelectedIndex--;
			listBox2.EndUpdate();
		}

		private void button11_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return;
			string higherValueToReplaceWith = String.Copy(localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count]);
			string lowerValueToReplaceWith = String.Copy(localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count + 1]);
			localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count] = lowerValueToReplaceWith;
			localSongs.fileNames[listBox2.SelectedIndex + globalSongs.fileNames.Count + 1] = higherValueToReplaceWith;


			listBox2.BeginUpdate();
			refreshListBoxes();
			listBox2.SelectedIndex++;
			listBox2.EndUpdate();
		}

		private void button8_Click(object sender, EventArgs e)
		{
			if (listBox2.SelectedIndex == -1) return;
			localSongs.fileNames.RemoveAt(listBox2.SelectedIndex + globalSongs.fileNames.Count);
			refreshListBoxes();
		}

		private void listBox2_DragDrop(object sender, DragEventArgs e)
		{
			const int ctrlKey = 8;

			if (e.Data.GetDataPresent(DataFormats.FileDrop))
			{
				string[] filePaths = (string[])(e.Data.GetData(DataFormats.FileDrop));
				for (int i = 0; i < filePaths.Length; i++)
				{
					string fileLocation = filePaths[i];

					int index = listBox2.SelectedIndex;
					if (index == -1)
						index = 0;

					if ((e.KeyState & ctrlKey) == ctrlKey && listBox2.Items.Count > 0)
					{
						localSongs.fileNames[index + i + globalSongs.fileNames.Count] = toRelativePath(fileLocation).Replace('\\', '/');
					}
					else
					{
						localSongs.fileNames.Insert(index + i + globalSongs.fileNames.Count + 1, toRelativePath(fileLocation).Replace('\\', '/'));
					}
				}

				refreshListBoxes();
			}

		}

		private void listBox2_DragEnter(object sender, DragEventArgs e)
		{
			if (e.Data.GetDataPresent(DataFormats.FileDrop))
			{
				string[] filePaths = (string[])(e.Data.GetData(DataFormats.FileDrop));
				foreach (string s in filePaths)
				{
					if (System.IO.Path.GetExtension(s) != ".txt")
					{
						e.Effect = DragDropEffects.None;
						return;
					}
				}
				e.Effect = DragDropEffects.Move;
			}
			else
			{
				e.Effect = DragDropEffects.None;
			}
		}




	}


	public static class ListExtra
	{
		public static void Resize<T>(this List<T> list, int sz, T c)
		{
			int cur = list.Count;
			if (sz < cur)
				list.RemoveRange(sz, cur - sz);
			else if (sz > cur)
			{
				if (sz > list.Capacity)//this bit is purely an optimisation, to avoid multiple automatic capacity changes.
					list.Capacity = sz;
				list.AddRange(Enumerable.Repeat(c, sz - cur));
			}
		}
		public static void Resize<T>(this List<T> list, int sz) where T : new()
		{
			Resize(list, sz, new T());
		}
	}




	public class SongList
	{
		public List<string> fileNames = new List<string>();
		string name = "";

		public static SongList parseSongs(string str, string labelname)
		{
			string[] lines = str.Split('\n');
			bool foundList = false;
			SongList ret = new SongList();

			foreach (string l in lines)
			{

				if (l.StartsWith(labelname + ":"))
				{
					foundList = true;
					continue;
				}
				else if (l.Contains(":"))
				{
					foundList = false;
				}

				if (foundList && l.Length > 2)
				{
					int number = Convert.ToInt32(l.Substring(0, 2), 16);
					string filePath = l.Substring(2).Trim();

					if (ret.fileNames.Count() <= number)
						ret.fileNames.Resize(number + 1, "");

					ret.fileNames[number] = filePath;
				}
				
			}
			ret.name = labelname;
			return ret;
		}

		public override string ToString()
		{
			string ret = name + ":" + System.Environment.NewLine;
			for (int i = 0; i < fileNames.Count; i++)
			{
				if (fileNames[i] != "")
				{
					ret += i.ToString("X2") + "  " + fileNames[i] + System.Environment.NewLine;
				}
			}
			return ret;
		}
	}


}



