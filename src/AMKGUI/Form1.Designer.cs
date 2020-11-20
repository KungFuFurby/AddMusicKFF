namespace AMKGUI
{
	partial class Form1
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.openListOpenFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.listBox1 = new System.Windows.Forms.ListBox();
			this.label1 = new System.Windows.Forms.Label();
			this.listBox2 = new System.Windows.Forms.ListBox();
			this.label2 = new System.Windows.Forms.Label();
			this.button1 = new System.Windows.Forms.Button();
			this.button2 = new System.Windows.Forms.Button();
			this.button3 = new System.Windows.Forms.Button();
			this.button4 = new System.Windows.Forms.Button();
			this.openSongOpenFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.button5 = new System.Windows.Forms.Button();
			this.romOpenFileDialog = new System.Windows.Forms.OpenFileDialog();
			this.button6 = new System.Windows.Forms.Button();
			this.timer1 = new System.Windows.Forms.Timer(this.components);
			this.verboseAMKCheckBox = new System.Windows.Forms.CheckBox();
			this.porterModeCheckBox = new System.Windows.Forms.CheckBox();
			this.splitContainer1 = new System.Windows.Forms.SplitContainer();
			this.splitContainer2 = new System.Windows.Forms.SplitContainer();
			this.label3 = new System.Windows.Forms.Label();
			this.button12 = new System.Windows.Forms.Button();
			this.button11 = new System.Windows.Forms.Button();
			this.button8 = new System.Windows.Forms.Button();
			this.checkBox1 = new System.Windows.Forms.CheckBox();
			this.button7 = new System.Windows.Forms.Button();
			this.textBox1 = new System.Windows.Forms.TextBox();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
			this.splitContainer1.Panel1.SuspendLayout();
			this.splitContainer1.Panel2.SuspendLayout();
			this.splitContainer1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).BeginInit();
			this.splitContainer2.Panel1.SuspendLayout();
			this.splitContainer2.Panel2.SuspendLayout();
			this.splitContainer2.SuspendLayout();
			this.SuspendLayout();
			// 
			// openListOpenFileDialog
			// 
			this.openListOpenFileDialog.FileName = "Addmusic_list.txt";
			this.openListOpenFileDialog.Filter = "Text files|*.txt";
			this.openListOpenFileDialog.Title = "Locate Addmusic_list.txt...";
			// 
			// listBox1
			// 
			this.listBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.listBox1.FormattingEnabled = true;
			this.listBox1.Location = new System.Drawing.Point(3, 28);
			this.listBox1.Name = "listBox1";
			this.listBox1.Size = new System.Drawing.Size(472, 121);
			this.listBox1.TabIndex = 0;
			this.listBox1.SelectedIndexChanged += new System.EventHandler(this.listBox1_SelectedIndexChanged);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 12);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(71, 13);
			this.label1.TabIndex = 1;
			this.label1.Text = "Global songs:";
			// 
			// listBox2
			// 
			this.listBox2.AllowDrop = true;
			this.listBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.listBox2.FormattingEnabled = true;
			this.listBox2.Location = new System.Drawing.Point(3, 168);
			this.listBox2.Name = "listBox2";
			this.listBox2.Size = new System.Drawing.Size(472, 186);
			this.listBox2.TabIndex = 1;
			this.listBox2.SelectedIndexChanged += new System.EventHandler(this.listBox2_SelectedIndexChanged);
			this.listBox2.DragDrop += new System.Windows.Forms.DragEventHandler(this.listBox2_DragDrop);
			this.listBox2.DragEnter += new System.Windows.Forms.DragEventHandler(this.listBox2_DragEnter);
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(3, 152);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(67, 13);
			this.label2.TabIndex = 1;
			this.label2.Text = "Local songs:";
			// 
			// button1
			// 
			this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button1.Enabled = false;
			this.button1.Location = new System.Drawing.Point(45, 22);
			this.button1.Name = "button1";
			this.button1.Size = new System.Drawing.Size(132, 23);
			this.button1.TabIndex = 0;
			this.button1.Text = "Change selected song";
			this.button1.UseVisualStyleBackColor = true;
			this.button1.Click += new System.EventHandler(this.button1_Click);
			// 
			// button2
			// 
			this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button2.Enabled = false;
			this.button2.Location = new System.Drawing.Point(44, 168);
			this.button2.Name = "button2";
			this.button2.Size = new System.Drawing.Size(133, 23);
			this.button2.TabIndex = 1;
			this.button2.Text = "Change selected song";
			this.button2.UseVisualStyleBackColor = true;
			this.button2.Click += new System.EventHandler(this.button2_Click);
			// 
			// button3
			// 
			this.button3.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button3.Enabled = false;
			this.button3.Location = new System.Drawing.Point(44, 197);
			this.button3.Name = "button3";
			this.button3.Size = new System.Drawing.Size(133, 23);
			this.button3.TabIndex = 2;
			this.button3.Text = "Clear selected song";
			this.button3.UseVisualStyleBackColor = true;
			this.button3.Click += new System.EventHandler(this.button3_Click);
			// 
			// button4
			// 
			this.button4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button4.Enabled = false;
			this.button4.Location = new System.Drawing.Point(44, 256);
			this.button4.Name = "button4";
			this.button4.Size = new System.Drawing.Size(133, 23);
			this.button4.TabIndex = 3;
			this.button4.Text = "Add new song";
			this.button4.UseVisualStyleBackColor = true;
			this.button4.Click += new System.EventHandler(this.button4_Click);
			// 
			// openSongOpenFileDialog
			// 
			this.openSongOpenFileDialog.Filter = "Text files|*.txt";
			this.openSongOpenFileDialog.Title = "Choose a song...";
			// 
			// button5
			// 
			this.button5.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.button5.Location = new System.Drawing.Point(204, 381);
			this.button5.Name = "button5";
			this.button5.Size = new System.Drawing.Size(71, 23);
			this.button5.TabIndex = 2;
			this.button5.Text = "Run";
			this.button5.UseVisualStyleBackColor = true;
			this.button5.Click += new System.EventHandler(this.button5_Click);
			// 
			// romOpenFileDialog
			// 
			this.romOpenFileDialog.Filter = "ROM files|*.smc; *.sfc";
			this.romOpenFileDialog.Title = "Select your ROM...";
			// 
			// button6
			// 
			this.button6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button6.Enabled = false;
			this.button6.Location = new System.Drawing.Point(44, 292);
			this.button6.Name = "button6";
			this.button6.Size = new System.Drawing.Size(133, 23);
			this.button6.TabIndex = 4;
			this.button6.Text = "Play this song";
			this.button6.UseVisualStyleBackColor = true;
			this.button6.Click += new System.EventHandler(this.button6_Click);
			// 
			// timer1
			// 
			this.timer1.Enabled = true;
			this.timer1.Interval = 10;
			this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
			// 
			// verboseAMKCheckBox
			// 
			this.verboseAMKCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.verboseAMKCheckBox.AutoSize = true;
			this.verboseAMKCheckBox.Location = new System.Drawing.Point(11, 357);
			this.verboseAMKCheckBox.Name = "verboseAMKCheckBox";
			this.verboseAMKCheckBox.Size = new System.Drawing.Size(91, 17);
			this.verboseAMKCheckBox.TabIndex = 7;
			this.verboseAMKCheckBox.Text = "Verbose AMK";
			this.verboseAMKCheckBox.UseVisualStyleBackColor = true;
			// 
			// porterModeCheckBox
			// 
			this.porterModeCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.porterModeCheckBox.AutoSize = true;
			this.porterModeCheckBox.Location = new System.Drawing.Point(11, 380);
			this.porterModeCheckBox.Name = "porterModeCheckBox";
			this.porterModeCheckBox.Size = new System.Drawing.Size(83, 17);
			this.porterModeCheckBox.TabIndex = 6;
			this.porterModeCheckBox.Text = "Porter mode";
			this.porterModeCheckBox.UseVisualStyleBackColor = true;
			this.porterModeCheckBox.CheckedChanged += new System.EventHandler(this.checkBox1_CheckedChanged);
			// 
			// splitContainer1
			// 
			this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.splitContainer1.IsSplitterFixed = true;
			this.splitContainer1.Location = new System.Drawing.Point(0, 0);
			this.splitContainer1.Name = "splitContainer1";
			this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			this.splitContainer1.Panel1.Controls.Add(this.splitContainer2);
			this.splitContainer1.Panel1MinSize = 400;
			// 
			// splitContainer1.Panel2
			// 
			this.splitContainer1.Panel2.Controls.Add(this.textBox1);
			this.splitContainer1.Panel2MinSize = 0;
			this.splitContainer1.Size = new System.Drawing.Size(671, 457);
			this.splitContainer1.SplitterDistance = 428;
			this.splitContainer1.TabIndex = 8;
			// 
			// splitContainer2
			// 
			this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.splitContainer2.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
			this.splitContainer2.Location = new System.Drawing.Point(0, 0);
			this.splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			this.splitContainer2.Panel1.Controls.Add(this.label1);
			this.splitContainer2.Panel1.Controls.Add(this.label3);
			this.splitContainer2.Panel1.Controls.Add(this.label2);
			this.splitContainer2.Panel1.Controls.Add(this.listBox2);
			this.splitContainer2.Panel1.Controls.Add(this.button5);
			this.splitContainer2.Panel1.Controls.Add(this.listBox1);
			// 
			// splitContainer2.Panel2
			// 
			this.splitContainer2.Panel2.Controls.Add(this.button12);
			this.splitContainer2.Panel2.Controls.Add(this.button11);
			this.splitContainer2.Panel2.Controls.Add(this.button8);
			this.splitContainer2.Panel2.Controls.Add(this.checkBox1);
			this.splitContainer2.Panel2.Controls.Add(this.button7);
			this.splitContainer2.Panel2.Controls.Add(this.button1);
			this.splitContainer2.Panel2.Controls.Add(this.button3);
			this.splitContainer2.Panel2.Controls.Add(this.verboseAMKCheckBox);
			this.splitContainer2.Panel2.Controls.Add(this.porterModeCheckBox);
			this.splitContainer2.Panel2.Controls.Add(this.button2);
			this.splitContainer2.Panel2.Controls.Add(this.button4);
			this.splitContainer2.Panel2.Controls.Add(this.button6);
			this.splitContainer2.Size = new System.Drawing.Size(671, 428);
			this.splitContainer2.SplitterDistance = 478;
			this.splitContainer2.TabIndex = 9;
			// 
			// label3
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
			this.label3.Location = new System.Drawing.Point(124, 407);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(230, 18);
			this.label3.TabIndex = 8;
			this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// button12
			// 
			this.button12.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button12.Enabled = false;
			this.button12.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.button12.Location = new System.Drawing.Point(5, 192);
			this.button12.Name = "button12";
			this.button12.Size = new System.Drawing.Size(33, 30);
			this.button12.TabIndex = 10;
			this.button12.Text = "⇑";
			this.button12.UseVisualStyleBackColor = true;
			this.button12.Click += new System.EventHandler(this.button12_Click);
			// 
			// button11
			// 
			this.button11.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button11.Enabled = false;
			this.button11.Font = new System.Drawing.Font("Arial", 15.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.button11.Location = new System.Drawing.Point(5, 228);
			this.button11.Name = "button11";
			this.button11.Size = new System.Drawing.Size(33, 30);
			this.button11.TabIndex = 10;
			this.button11.Text = "⇓";
			this.button11.UseVisualStyleBackColor = true;
			this.button11.Click += new System.EventHandler(this.button11_Click);
			// 
			// button8
			// 
			this.button8.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.button8.Enabled = false;
			this.button8.Location = new System.Drawing.Point(44, 227);
			this.button8.Name = "button8";
			this.button8.Size = new System.Drawing.Size(133, 23);
			this.button8.TabIndex = 9;
			this.button8.Text = "Remove selected song";
			this.button8.UseVisualStyleBackColor = true;
			this.button8.Click += new System.EventHandler(this.button8_Click);
			// 
			// checkBox1
			// 
			this.checkBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.checkBox1.AutoSize = true;
			this.checkBox1.Checked = true;
			this.checkBox1.CheckState = System.Windows.Forms.CheckState.Checked;
			this.checkBox1.Location = new System.Drawing.Point(11, 403);
			this.checkBox1.Name = "checkBox1";
			this.checkBox1.Size = new System.Drawing.Size(91, 17);
			this.checkBox1.TabIndex = 8;
			this.checkBox1.Text = "Autoplay SPC";
			this.checkBox1.UseVisualStyleBackColor = true;
			this.checkBox1.Visible = false;
			// 
			// button7
			// 
			this.button7.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.button7.Location = new System.Drawing.Point(10, 328);
			this.button7.Name = "button7";
			this.button7.Size = new System.Drawing.Size(133, 23);
			this.button7.TabIndex = 5;
			this.button7.Text = "Reload list.txt from disk";
			this.button7.UseVisualStyleBackColor = true;
			this.button7.Click += new System.EventHandler(this.button7_Click);
			// 
			// textBox1
			// 
			this.textBox1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.textBox1.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
			this.textBox1.Location = new System.Drawing.Point(0, 0);
			this.textBox1.MaxLength = 0;
			this.textBox1.Multiline = true;
			this.textBox1.Name = "textBox1";
			this.textBox1.ReadOnly = true;
			this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.textBox1.Size = new System.Drawing.Size(671, 25);
			this.textBox1.TabIndex = 0;
			// 
			// Form1
			// 
			this.AcceptButton = this.button5;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(671, 457);
			this.Controls.Add(this.splitContainer1);
			this.MinimumSize = new System.Drawing.Size(596, 496);
			this.Name = "Form1";
			this.Text = "AMKGUI";
			this.Load += new System.EventHandler(this.Form1_Load);
			this.Shown += new System.EventHandler(this.Form1_Shown);
			this.Resize += new System.EventHandler(this.Form1_Resize);
			this.splitContainer1.Panel1.ResumeLayout(false);
			this.splitContainer1.Panel2.ResumeLayout(false);
			this.splitContainer1.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
			this.splitContainer1.ResumeLayout(false);
			this.splitContainer2.Panel1.ResumeLayout(false);
			this.splitContainer2.Panel1.PerformLayout();
			this.splitContainer2.Panel2.ResumeLayout(false);
			this.splitContainer2.Panel2.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.splitContainer2)).EndInit();
			this.splitContainer2.ResumeLayout(false);
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.OpenFileDialog openListOpenFileDialog;
		private System.Windows.Forms.ListBox listBox1;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.ListBox listBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button button1;
		private System.Windows.Forms.Button button2;
		private System.Windows.Forms.Button button3;
		private System.Windows.Forms.Button button4;
		private System.Windows.Forms.OpenFileDialog openSongOpenFileDialog;
		private System.Windows.Forms.Button button5;
		private System.Windows.Forms.OpenFileDialog romOpenFileDialog;
		private System.Windows.Forms.Button button6;
		private System.Windows.Forms.Timer timer1;
		private System.Windows.Forms.CheckBox verboseAMKCheckBox;
		private System.Windows.Forms.CheckBox porterModeCheckBox;
		private System.Windows.Forms.SplitContainer splitContainer1;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.SplitContainer splitContainer2;
		private System.Windows.Forms.Button button7;
		private System.Windows.Forms.CheckBox checkBox1;
		private System.Windows.Forms.Button button12;
		private System.Windows.Forms.Button button11;
		private System.Windows.Forms.Button button8;
	}
}

