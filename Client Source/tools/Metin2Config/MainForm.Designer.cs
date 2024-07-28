namespace Config
{
    partial class MainForm
    {
        /// <summary>
        /// Variabile di progettazione necessaria.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Liberare le risorse in uso.
        /// </summary>
        /// <param name="disposing">ha valore true se le risorse gestite devono essere eliminate, false in caso contrario.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Codice generato da Progettazione Windows Form

        /// <summary>
        /// Metodo necessario per il supporto della finestra di progettazione. Non modificare
        /// il contenuto del metodo con l'editor di codice.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.toolTip = new System.Windows.Forms.ToolTip(this.components);
            this.checkShowDmg = new System.Windows.Forms.CheckBox();
            this.checkShowChat = new System.Windows.Forms.CheckBox();
            this.trackMusic = new System.Windows.Forms.TrackBar();
            this.labelSFX = new System.Windows.Forms.Label();
            this.trackSFX = new System.Windows.Forms.TrackBar();
            this.labelMusic = new System.Windows.Forms.Label();
            this.comboResolution = new System.Windows.Forms.ComboBox();
            this.labelResolution = new System.Windows.Forms.Label();
            this.checkWindowed = new System.Windows.Forms.CheckBox();
            this.comboFog = new System.Windows.Forms.ComboBox();
            this.labelFog = new System.Windows.Forms.Label();
            this.comboShadows = new System.Windows.Forms.ComboBox();
            this.labelShadows = new System.Windows.Forms.Label();
            this.GFXContainer = new System.Windows.Forms.GroupBox();
            this.audioContainer = new System.Windows.Forms.GroupBox();
            this.GFXContainer2 = new System.Windows.Forms.GroupBox();
            this.gameContainer = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.trackMusic)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackSFX)).BeginInit();
            this.GFXContainer.SuspendLayout();
            this.audioContainer.SuspendLayout();
            this.GFXContainer2.SuspendLayout();
            this.gameContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // toolTip
            // 
            this.toolTip.AutoPopDelay = 5000;
            this.toolTip.InitialDelay = 0;
            this.toolTip.ReshowDelay = 100;
            // 
            // checkShowDmg
            // 
            this.checkShowDmg.AutoSize = true;
            this.checkShowDmg.Location = new System.Drawing.Point(12, 53);
            this.checkShowDmg.Name = "checkShowDmg";
            this.checkShowDmg.Size = new System.Drawing.Size(86, 18);
            this.checkShowDmg.TabIndex = 1;
            this.checkShowDmg.Text = "Hasar Bilgisi";
            this.toolTip.SetToolTip(this.checkShowDmg, "Hasar Bilgisini Gösterir");
            this.checkShowDmg.UseVisualStyleBackColor = true;
            this.checkShowDmg.CheckedChanged += new System.EventHandler(this.CheckShowDmgCheckedChanged);
            // 
            // checkShowChat
            // 
            this.checkShowChat.AutoSize = true;
            this.checkShowChat.Location = new System.Drawing.Point(12, 29);
            this.checkShowChat.Name = "checkShowChat";
            this.checkShowChat.Size = new System.Drawing.Size(96, 18);
            this.checkShowChat.TabIndex = 2;
            this.checkShowChat.Text = "Sohbet Satırı";
            this.toolTip.SetToolTip(this.checkShowChat, "Sohbet Satırını Gösterir");
            this.checkShowChat.UseVisualStyleBackColor = true;
            this.checkShowChat.CheckedChanged += new System.EventHandler(this.CheckShowChatCheckedChanged);
            // 
            // trackMusic
            // 
            this.trackMusic.AutoSize = false;
            this.trackMusic.LargeChange = 2;
            this.trackMusic.Location = new System.Drawing.Point(9, 88);
            this.trackMusic.Maximum = 500;
            this.trackMusic.Name = "trackMusic";
            this.trackMusic.Size = new System.Drawing.Size(283, 32);
            this.trackMusic.TabIndex = 0;
            this.trackMusic.TickFrequency = 100;
            this.trackMusic.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
            this.toolTip.SetToolTip(this.trackMusic, "Oyun Müziği");
            this.trackMusic.Scroll += new System.EventHandler(this.TrackMusicScroll);
            // 
            // labelSFX
            // 
            this.labelSFX.AutoSize = true;
            this.labelSFX.Location = new System.Drawing.Point(6, 19);
            this.labelSFX.Name = "labelSFX";
            this.labelSFX.Size = new System.Drawing.Size(49, 14);
            this.labelSFX.TabIndex = 1;
            this.labelSFX.Text = "Efektler";
            this.toolTip.SetToolTip(this.labelSFX, "Ses Efekt Düzeyi");
            // 
            // trackSFX
            // 
            this.trackSFX.AutoSize = false;
            this.trackSFX.LargeChange = 2;
            this.trackSFX.Location = new System.Drawing.Point(9, 36);
            this.trackSFX.Maximum = 5;
            this.trackSFX.Name = "trackSFX";
            this.trackSFX.Size = new System.Drawing.Size(283, 27);
            this.trackSFX.TabIndex = 2;
            this.trackSFX.TickStyle = System.Windows.Forms.TickStyle.TopLeft;
            this.toolTip.SetToolTip(this.trackSFX, "Ses Efektleri");
            this.trackSFX.Scroll += new System.EventHandler(this.TrackSfxScroll);
            // 
            // labelMusic
            // 
            this.labelMusic.AutoSize = true;
            this.labelMusic.Location = new System.Drawing.Point(6, 71);
            this.labelMusic.Name = "labelMusic";
            this.labelMusic.Size = new System.Drawing.Size(36, 14);
            this.labelMusic.TabIndex = 3;
            this.labelMusic.Text = "Müzik";
            this.toolTip.SetToolTip(this.labelMusic, "Oyun Müziği Düzeyi");
            // 
            // comboResolution
            // 
            this.comboResolution.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboResolution.FormattingEnabled = true;
            this.comboResolution.Location = new System.Drawing.Point(9, 39);
            this.comboResolution.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.comboResolution.Name = "comboResolution";
            this.comboResolution.Size = new System.Drawing.Size(255, 22);
            this.comboResolution.TabIndex = 0;
            this.toolTip.SetToolTip(this.comboResolution, "Oyun Çözünürlüğü");
            this.comboResolution.SelectedIndexChanged += new System.EventHandler(this.ComboResolutionSelectedIndexChanged);
            // 
            // labelResolution
            // 
            this.labelResolution.AutoSize = true;
            this.labelResolution.Location = new System.Drawing.Point(6, 20);
            this.labelResolution.Name = "labelResolution";
            this.labelResolution.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.labelResolution.Size = new System.Drawing.Size(66, 14);
            this.labelResolution.TabIndex = 1;
            this.labelResolution.Text = "Çözünürlük";
            this.toolTip.SetToolTip(this.labelResolution, "Pencere Boyutu");
            // 
            // checkWindowed
            // 
            this.checkWindowed.AutoSize = true;
            this.checkWindowed.Location = new System.Drawing.Point(9, 68);
            this.checkWindowed.Name = "checkWindowed";
            this.checkWindowed.Size = new System.Drawing.Size(105, 18);
            this.checkWindowed.TabIndex = 19;
            this.checkWindowed.Text = "Pencere Modu";
            this.toolTip.SetToolTip(this.checkWindowed, "Tam Ekran/Pencere");
            this.checkWindowed.UseVisualStyleBackColor = true;
            this.checkWindowed.CheckedChanged += new System.EventHandler(this.CheckFullscreenCheckedChanged);
            // 
            // comboFog
            // 
            this.comboFog.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboFog.FormattingEnabled = true;
            this.comboFog.Items.AddRange(new object[] {
            "Yakın",
            "Orta",
            "Uzak"});
            this.comboFog.Location = new System.Drawing.Point(6, 40);
            this.comboFog.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.comboFog.Name = "comboFog";
            this.comboFog.Size = new System.Drawing.Size(147, 22);
            this.comboFog.TabIndex = 14;
            this.toolTip.SetToolTip(this.comboFog, "Harita Sisi");
            this.comboFog.SelectedIndexChanged += new System.EventHandler(this.ComboFogSelectedIndexChanged);
            // 
            // labelFog
            // 
            this.labelFog.AutoSize = true;
            this.labelFog.Location = new System.Drawing.Point(6, 20);
            this.labelFog.Name = "labelFog";
            this.labelFog.Size = new System.Drawing.Size(21, 14);
            this.labelFog.TabIndex = 15;
            this.labelFog.Text = "Sis";
            this.toolTip.SetToolTip(this.labelFog, "Harita Sisini Ayarlar");
            // 
            // comboShadows
            // 
            this.comboShadows.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.comboShadows.FormattingEnabled = true;
            this.comboShadows.Items.AddRange(new object[] {
            "Kapalı",
            "Düşük Kalite",
            "Normal Kalite",
            "Yüksek Kalite",
            "Ultra Kalite"});
            this.comboShadows.Location = new System.Drawing.Point(6, 88);
            this.comboShadows.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.comboShadows.Name = "comboShadows";
            this.comboShadows.Size = new System.Drawing.Size(147, 22);
            this.comboShadows.TabIndex = 16;
            this.toolTip.SetToolTip(this.comboShadows, "Gölgeler");
            this.comboShadows.SelectedIndexChanged += new System.EventHandler(this.ComboShadowsSelectedIndexChanged);
            // 
            // labelShadows
            // 
            this.labelShadows.AutoSize = true;
            this.labelShadows.Location = new System.Drawing.Point(6, 68);
            this.labelShadows.Name = "labelShadows";
            this.labelShadows.Size = new System.Drawing.Size(51, 14);
            this.labelShadows.TabIndex = 17;
            this.labelShadows.Text = "Gölgeler";
            this.toolTip.SetToolTip(this.labelShadows, "Gölgeler");
            // 
            // GFXContainer
            // 
            this.GFXContainer.Controls.Add(this.comboResolution);
            this.GFXContainer.Controls.Add(this.labelResolution);
            this.GFXContainer.Controls.Add(this.checkWindowed);
            this.GFXContainer.Location = new System.Drawing.Point(12, 12);
            this.GFXContainer.Name = "GFXContainer";
            this.GFXContainer.Size = new System.Drawing.Size(298, 110);
            this.GFXContainer.TabIndex = 2;
            this.GFXContainer.TabStop = false;
            this.GFXContainer.Text = "Görünüm";
            // 
            // audioContainer
            // 
            this.audioContainer.Controls.Add(this.trackSFX);
            this.audioContainer.Controls.Add(this.labelMusic);
            this.audioContainer.Controls.Add(this.trackMusic);
            this.audioContainer.Controls.Add(this.labelSFX);
            this.audioContainer.Location = new System.Drawing.Point(12, 128);
            this.audioContainer.Name = "audioContainer";
            this.audioContainer.Size = new System.Drawing.Size(298, 135);
            this.audioContainer.TabIndex = 3;
            this.audioContainer.TabStop = false;
            this.audioContainer.Text = "Ses";
            this.audioContainer.Enter += new System.EventHandler(this.audioContainer_Enter);
            // 
            // GFXContainer2
            // 
            this.GFXContainer2.Controls.Add(this.labelFog);
            this.GFXContainer2.Controls.Add(this.comboFog);
            this.GFXContainer2.Controls.Add(this.labelShadows);
            this.GFXContainer2.Controls.Add(this.comboShadows);
            this.GFXContainer2.Location = new System.Drawing.Point(322, 12);
            this.GFXContainer2.Name = "GFXContainer2";
            this.GFXContainer2.Size = new System.Drawing.Size(179, 144);
            this.GFXContainer2.TabIndex = 19;
            this.GFXContainer2.TabStop = false;
            this.GFXContainer2.Text = "Grafikler";
            // 
            // gameContainer
            // 
            this.gameContainer.Controls.Add(this.checkShowDmg);
            this.gameContainer.Controls.Add(this.checkShowChat);
            this.gameContainer.Location = new System.Drawing.Point(322, 164);
            this.gameContainer.Name = "gameContainer";
            this.gameContainer.Size = new System.Drawing.Size(179, 99);
            this.gameContainer.TabIndex = 27;
            this.gameContainer.TabStop = false;
            this.gameContainer.Text = "Oyun";
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.ClientSize = new System.Drawing.Size(523, 286);
            this.Controls.Add(this.gameContainer);
            this.Controls.Add(this.GFXContainer2);
            this.Controls.Add(this.audioContainer);
            this.Controls.Add(this.GFXContainer);
            this.Font = new System.Drawing.Font("Tahoma", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Margin = new System.Windows.Forms.Padding(3, 4, 3, 4);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Oyun Seçenekleri";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.OnFormClosing);
            this.Load += new System.EventHandler(this.Form1Load);
            ((System.ComponentModel.ISupportInitialize)(this.trackMusic)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.trackSFX)).EndInit();
            this.GFXContainer.ResumeLayout(false);
            this.GFXContainer.PerformLayout();
            this.audioContainer.ResumeLayout(false);
            this.audioContainer.PerformLayout();
            this.GFXContainer2.ResumeLayout(false);
            this.GFXContainer2.PerformLayout();
            this.gameContainer.ResumeLayout(false);
            this.gameContainer.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ToolTip toolTip;
        private System.Windows.Forms.CheckBox checkShowChat;
        private System.Windows.Forms.CheckBox checkShowDmg;
        private System.Windows.Forms.Label labelMusic;
        private System.Windows.Forms.TrackBar trackSFX;
        private System.Windows.Forms.Label labelSFX;
        private System.Windows.Forms.TrackBar trackMusic;
        private System.Windows.Forms.CheckBox checkWindowed;
        private System.Windows.Forms.Label labelResolution;
        private System.Windows.Forms.ComboBox comboResolution;
        private System.Windows.Forms.Label labelShadows;
        private System.Windows.Forms.ComboBox comboShadows;
        private System.Windows.Forms.Label labelFog;
        private System.Windows.Forms.ComboBox comboFog;
        private System.Windows.Forms.GroupBox GFXContainer;
        private System.Windows.Forms.GroupBox audioContainer;
        private System.Windows.Forms.GroupBox GFXContainer2;
        private System.Windows.Forms.GroupBox gameContainer;
    }
}

